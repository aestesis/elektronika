/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						"elektroexp.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	BPMTRACKER.CPP				(c)	YoY'04						WEB: www.aestesis.org
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include						<stdio.h>
#include						<assert.h>
#include						<math.h>
#include						"bpmTracker.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL ACI						AbpmTracker::CI=ACI("AbpmTracker", GUID(0xE4EC7600,0x00000500), &Anode::CI, 0, NULL);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	-- ALGORYTHME FROM --
//
//  "automatic extraction of tempo and beat expressive performance" 
//
//  by simon dixon
//  email: simon@oefai.at
//
//  austrian research institute for artificial intelligence
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define							WINDOW							882			// => 20 milliseconds
#define							STEP							441			// => 10 milliseconds
#define							SILENCE							1024		// moyenne minimum du son (=> pas de son)
#define							NBCLUSTER						200
#define							MAXBEATAGENTS					1000
#define							TIMEOUT							(110<<16)	// 1,10 secondes	FIXEDINT

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	int		interval;	// beat time (interval)
	int		prev;		// previous beat
	int		next;		// next beat
	int		score;		// scoring
	int		cluster;	// # cluster
} Agent;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AbpmTracker::AbpmTracker(char *name) : Anode(name)
{
	memset(envelope, 0, sizeof(envelope));
	epos=0;
	soffset=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL AbpmTracker::~AbpmTracker()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL float AbpmTracker::getBPM(Asample *smp)
{
	float	bpm=0.f;
	smp->enter(__FILE__,__LINE__);
	{
		if(smp->offset<soffset)
		{
			compute(smp->medium, smp->treble, soffset, smp->size, smp->size);
			compute(smp->medium, smp->treble, 0, smp->offset, smp->size);
			soffset=smp->offset;
		}
		else
		{
			compute(smp->medium, smp->treble, soffset, smp->offset, smp->size);
			soffset=smp->offset;
		}
	}
	smp->leave();
	if(epos>=ENVELOPESIZE)
	{
		bool	peak[ENVELOPESIZE];
		int		i;
		double	average=0.0;
		
		memset(peak, 0, sizeof(peak));
		
		for(i=0; i<ENVELOPESIZE; i++)
			average+=envelope[i];
		average/=(double)ENVELOPESIZE;
		
		if(average>SILENCE)	
		{
			for(i=2; i<ENVELOPESIZE-2; i++)
			{
				double	v=envelope[i];
				if((v>average)&&(v>envelope[i-1])&&(v>envelope[i-2])&&(v>envelope[i+1])&&(v>envelope[i+1]))
					peak[i]=true;
			}
			
			
			{	// cluster
				int		cluster[NBCLUSTER][ENVELOPESIZE];		// 200 == 2 secondes
				int		iclust[NBCLUSTER];						// sort index
				int		nclust[NBCLUSTER];
				int		sclust[NBCLUSTER];						// cluster scoring
				int		i,j;
				
				memset(cluster, 0, sizeof(cluster));
				memset(nclust, 0, sizeof(nclust));
				memset(iclust, 0, sizeof(nclust));
				memset(sclust, 0, sizeof(sclust));
				
				for(i=0; i<ENVELOPESIZE; i++)
				{
					if(peak[i])
					{
						int	nb=min(i+NBCLUSTER, ENVELOPESIZE);
						for(j=i+1; j<nb; j++)
							if(peak[j])
							{
								int	n=j-i-1;
								cluster[n][nclust[n]++]=i;
							}
					}
				}
				
				// scoring
				for(i=NBCLUSTER-1; i>=0; i--)
				{
					int	nf=i;
					for(j=2; j<=4; j++)
					{
						int	n=i/j;
						if(n==nf)
							break;
						sclust[i]+=nclust[n]*(6-j);
						nf=n;
					}
					nf=i;
					for(j=5; j<=8; j++)
					{
						int	n=i/j;
						if(n==nf)
							break;
						sclust[i]+=nclust[n];
						nf=n;
					}
					nf=i;
					for(j=2; j<=4; j++)
					{
						int	n=i*j;
						if((n>=NBCLUSTER)||(n==nf))
							break;
						sclust[i]+=nclust[n]*(6-j);
						nf=n;
					}
					nf=i;
					for(j=5; j<=8; j++)
					{
						int	n=i*j;
						if((n>=NBCLUSTER)||(n==nf))
							break;
						sclust[i]+=nclust[n];
						nf=n;
					}
					sclust[i]+=nclust[i]*5;
				}
				
				// sort clusters
				for(i=0; i<NBCLUSTER; i++)
					iclust[i]=i;
				for(i=0; i<NBCLUSTER; i++)
				{
					for(j=NBCLUSTER-1; j>i; j--)
					{
						if(sclust[iclust[j]]>sclust[iclust[i]])
						{
							int	v=iclust[i];
							iclust[i]=iclust[j];
							iclust[j]=v;
						}
					}
				}
				
				if(sclust[iclust[0]]>100)
					bpm=100*60.f/(float)(iclust[0]+1);
					
				/*
				{
					Agent	agents[MAXBEATAGENTS];
					int		nbagents=0;
					
					memset(agents, 0, sizeof(agents));
					
					for(i=0; i<16; i++)	// seulement les 16 meilleurs scoring clusters
					{
						int	ibeat=iclust[i]+1;	// beat interval
						for(j=0; j<ibeat; j++)
						{
							if(peak[j])
							{
								Agent	*pA=&agents[nbagents++];
								pA->cluster=iclust[i];
								pA->interval=ibeat<<16;		// FIXEDINT 16.16 bits
								pA->prev=j<<16;
								pA->next=pA->prev+pA->interval;
								pA->score=sclust[pA->cluster];
								if(nbagents>=MAXBEATAGENTS)
									goto label_max;
							}
						}
					}
				
				label_max:	;
				
					for(i=0; i<ENVELOPESIZE; i++)
					{
						if(peak[i])
						{
							int	onset=i<<16;
							for(j=0; j<nbagents; j++)
							{
								Agent	*pA=&agents[j];
								if(pA->interval)	// not deleted agent
								{
									//if(onset-pA->prev>TIMEOUT)
									//	pA->interval=0;	// delete agent
									//else
									{
										while(pA->next+(2<<16)<onset)	// error max==2 => 20 ms
											pA->next=pA->next+pA->interval;
										if((pA->next-(2<<16)<=onset)&&(onset<=pA->next+(2<<16)))
										{
											if(abs(pA->next-onset)>=(1<<16))
											{
												Agent	*pB=NULL;
												
												
												//{
												//	int	k;
												//	for(k=0; k<nbagents; k++)
												//		if(agents[k].interval==0)
												//			pB=&agents[k];
												//}
												
				
												
												if((!pB)&&(nbagents<MAXBEATAGENTS))
													pB=&agents[nbagents++];
												
												if(pB)
												{
													int		error=onset-pA->next;
													memcpy(pB, pA, sizeof(Agent));
													pB->interval=pB->interval+error/16;
													pB->next=onset+pB->interval;
													pB->prev=onset;
													int	sc=(int)((float)pB->score+(1-fabs((float)error*10.f)/(65536.f*2.f)));
													pB->score=sc;
												}
											}
											else
												pA->score++;
										}
									}
								}
							}
						}		
					}
					{
						Agent		*pMax=NULL;
						int					max=0;
						for(i=0; i<nbagents; i++)
						{
							Agent	*pA=&agents[i];
							if(pA->interval&&(pA->score>max))
							{
								max=pA->score;
								pMax=pA;
							}
						}
						if(max>100)
							bpm=100*60.f/((float)pMax->interval/65536.f);
					}
				}
				*/
			}
		}		
		epos=0;
	}
	
	return bpm;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double __inline calc(sword *medium, sword *treble, int p)
{
	int		n=p<<1;
	return fabs((double)medium[n]+(double)medium[n+1]+((double)treble[n]+(double)treble[n+1])*1.5f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDLL void AbpmTracker::compute(sword *medium, sword *treble, int begin, int end, int size)
{
	int	i,j;
	begin=(begin/STEP)*STEP;
	end=(end/STEP)*STEP;
	if(epos<ENVELOPESIZE)
	{
		for(i=begin; i<end; i+=STEP)
		{
			int		d=i-STEP;
			int		e=i+STEP;
			double	sum=0.0;
			if(d<0)
			{
				for(j=size-d; j<size; j++)
					sum+=calc(medium, treble, j);
				for(j=0; j<e; j++)
					sum+=calc(medium, treble, j);
			}
			else if(e>size)
			{
				for(j=d; j<size; j++)
					sum+=calc(medium, treble, j);
				for(j=0; j<e; j++)
					sum+=calc(medium, treble, j);
			}
			else
			{
				for(j=d; j<e; j++)
					sum+=calc(medium, treble, j);
			}
			envelope[epos++]=sum;
			if(epos>=ENVELOPESIZE)
				break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
