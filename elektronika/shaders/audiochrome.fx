///////////////////////////////////////////////////////////////////////////////
// (c) aestesis 2005 - code by renan jegouzo [aka YoY] - renan@aestesis.org
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// common input from elektronika

texture texture0;	// texture
texture	texsound;	// texure with audio sample

float4	color;		// rendering color
//float	beat;		// beat value [0..infinite] - 120bpm - ex: beat=120 after 1 mn elektronika running
//float	rbeat;		// relative beat value 	[0..1]  0..1..0..1..0.. etc..
//float	bass;		// bass audio level 	[0..1]
//float	medium;		// medium audio level 	[0..1]
//float	treble;		// treble audio level	[0..1]

///////////////////////////////////////////////////////////////////////////////
// effect special input (only float) elektronika assign a knob for each value (8 knobs max)

float	power;
float	red;
float	green;
float	blue;
float	lum;
float	ared;
float	agreen;
float	ablue;

///////////////////////////////////////////////////////////////////////////////
// effect global var

///////////////////////////////////////////////////////////////////////////////

struct PS_INPUT
{
	float2 vTexCoord: TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////

sampler2D SceneColorSampler = sampler_state
{
	texture = (texture0);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

///////////////////////////////////////////////////////////////////////////////

sampler2D SceneSoundSampler = sampler_state
{
	texture = (texsound);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float4 ps_filter( PS_INPUT v ) : COLOR
{
	float4	cori=tex2D( SceneColorSampler, v.vTexCoord);
	float4	c=cori;
	float2	p;
	float2	cx;
	float	i;

	p.y=0.33;
	p.x=cori.r;
	cx=tex2D( SceneSoundSampler, p);
	i=(cx.x+cx.y);
	c.r=c.r*lum+c.r*i*red*2+ared-0.5;
	

	p.y=0.66;
	p.x=cori.g*0.5;
	cx=tex2D( SceneSoundSampler, p);
	i=(cx.x+cx.y);
	c.g=c.g*lum+c.g*i*green*2+agreen-0.5;

	p.y=1;
	p.x=cori.b*0.1;
	cx=tex2D( SceneSoundSampler, p);
	i=(cx.x+cx.y);
	c.b=c.b*lum+c.b*i*blue*2+ablue-0.5;

	return 	((c*power)+cori*(1.f-power))*color;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

technique elektronika
{
	pass P0
	{
		PixelShader = compile ps_2_0 ps_filter();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
