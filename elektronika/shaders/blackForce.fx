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
	float4	ctex= tex2D(SceneColorSampler, v.vTexCoord);
    return 	ctex*ctex*color;
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
