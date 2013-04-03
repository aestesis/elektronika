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
float	treble;	// treble audio level	[0..1]

///////////////////////////////////////////////////////////////////////////////
// effect special input (only float) elektronika assign a knob for each value (8 knobs max)

float	power;
float	size;
float	luminosity;

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
  	const int NUM = 9;
   	const float threshold = 0.05;

   	const float2 c[NUM] = {
			float2(-0.0078125, 0.0078125), 
			float2( 0.00 ,     0.0078125),
			float2( 0.0078125, 0.0078125),
			float2(-0.0078125, 0.00 ),
			float2( 0.0,       0.0),
			float2( 0.0078125, 0.007 ),
			float2(-0.0078125,-0.0078125),
			float2( 0.00 ,    -0.0078125),
			float2( 0.0078125,-0.0078125),
    };

	float4 	col[NUM];    
	int 	i;

  	for (i=0; i < NUM; i++) 
	{
     	 col[i] = tex2D( SceneColorSampler, v.vTexCoord.xy + c[i] * size);
    	}

    float3 rgb2lum = float3(0.30, 0.59, 0.11);
    
	float lum[NUM];
   	for (i = 0; i < NUM; i++) 
	{
		lum[i] = dot(col[i].xyz, rgb2lum);
    }

    float x = lum[2]+  lum[8]+2*lum[5]-lum[0]-2*lum[3]-lum[6];
    float y = lum[6]+2*lum[7]+  lum[8]-lum[0]-2*lum[1]-lum[2];


	float edge =(x*x + y*y < threshold)? 1.0:(1.f-power);

	float4	ctex;

    ctex.rgb = luminosity * 2.f * col[5].xyz * edge.xxx;
    ctex.a = col[5].a;
	return ctex*color;
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
