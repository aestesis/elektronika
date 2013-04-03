///////////////////////////////////////////////////////////////////////////////
// (c) aestesis 2005 - code by renan jegouzo [aka YoY] - renan@aestesis.org
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// common input from elektronika

texture texture0;	// texture

float4	color;		// rendering color
float	beat;		// beat value [0..infinite] - 120bpm - ex: beat=120 after 1 mn elektronika running
float	rbeat;		// relative beat value 	[0..1]  0..1..0..1..0.. etc..
float	bass;		// bass audio level 	[0..1]
float	medium;		// medium audio level 	[0..1]
float	treble;		// treble audio level	[0..1]

///////////////////////////////////////////////////////////////////////////////
// effect special input (only float) elektronika assign a knob for each value (8 knobs max)

float	power;
float	speed;
float	size;

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

//float zygzyg(float i, float val)
//{
//	float	n=i*beat*1.3212120;
//	return sin((n+515.51515+beat*0.5963217123)*val)*sin((n+142.151212+beat*0.051561558)*val);
//}

float4 ps_filter( PS_INPUT v ) : COLOR
{
	float2	zyg;
	float	b=beat*(speed+0.01)*2.f;

	float	cc=(5.51515+b*0.05963217123);
	zyg.x=0.5+cos(cc)*sin((1.151212+b*0.051561558));
	zyg.y=0.5+sin(cc)*sin((2.151212+b*0.0514115120));

	float r=dot(v.vTexCoord, zyg)*30.f*size;

	float	l=(sin(r)*0.5f+0.5f)*0.1f*power;
	float2	p=v.vTexCoord*(1.f-l)+zyg*l;

//	p.x=(p.x+100.f)%1.f;	// not enought instruction in shader 2.0
//	p.y=(p.y+100.f)%1.f;

	return tex2D(SceneColorSampler, p)*color;
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
