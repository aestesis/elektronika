///////////////////////////////////////////////////////////////////////////////
// (c) aestesis 2003 - code by renan jegouzo [aka YoY] - renan@aestesis.org
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// comon input from elektronika

texture texture0;	// texture

float4	color;		// rendering color
float	beat;		// beat value [0..infinite] - 120bpm - ex: beat=120 after 1 mn elektronika running
float	rbeat;		// relative beat value 	[0..1]  0..1..0..1..0.. etc..
float	bass;		// bass audio level 	[0..1]
float	medium;		// medium audio level 	[0..1]
float	treble;	// treble audio level	[0..1]

///////////////////////////////////////////////////////////////////////////////
// effect special input (only float) elektronika assign a knob for each value (8 knobs max)

float	power;

///////////////////////////////////////////////////////////////////////////////
// effect global var

///////////////////////////////////////////////////////////////////////////////

struct PS_INPUT
{
	float2 vTexCoord: TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////

sampler SceneColorSampler = sampler_state
{
	texture = (texture0);
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float4 ps_filter( PS_INPUT v ) : COLOR
{
	float2	p,p2;

	p.x=cos(beat)*0.1*power;
	p.y=sin(beat*0.99511412)*0.1*power;

	p2.x=cos(beat*1.01852)*0.1*power;
	p2.y=sin(beat*0.97511412)*0.1*power;
	
	float4 	ctex0 = tex2D(SceneColorSampler, v.vTexCoord);
	float4	ctex1 = tex2D(SceneColorSampler, v.vTexCoord+p);
	float4	ctex2 = tex2D(SceneColorSampler, v.vTexCoord+p2);
	ctex0.y=ctex1.y;
	ctex0.z=ctex2.z;
	ctex0*=color;
    return 	ctex0;
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
