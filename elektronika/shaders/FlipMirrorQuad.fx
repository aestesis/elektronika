///////////////////////////////////////////////////////////////////////////////
// (c) aestesis 2003 - code by renan jegouzo [aka YoY] - renan@aestesis.org
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// comon input from elektronika

texture texture0;		// texture

float4 color;			// rendering color
//half	beat;			// beat value [0..infinite] - 120bpm - ex: beat=120 after 1 mn elektronika running
//half	rbeat;			// relative beat value 	[0..1]  0..1..0..1..0.. etc..
//half	bass;			// bass audio level 	[0..1]
//half	medium;			// medium audio level 	[0..1]
//half	treble;			// treble audio level	[0..1]

///////////////////////////////////////////////////////////////////////////////
// effect special input (only float) elektronika assign a knob for each value (8 knobs max)

half 	power; 			// Elek Power
half 	flipX;			// Elek FlipX		ON/OFF a 0.5
half 	flipY; 			// Elek FlipY		ON/OFF a 0.5
half 	quad;			// Elek Quad		5 valeurs
half 	quadmirror;		// Elek QuadMirror ON/OFF a 0.6
half 	lumakey;		// Elek LumaKey		ON/OFF a 0.6

///////////////////////////////////////////////////////////////////////////////

struct vs_out {
    half4 position  : POSITION;
    half2 texcoord0  : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////

sampler2D ImageSampler = sampler_state
{
	texture = (texture0);
    	MipFilter = LINEAR;
    	MinFilter = LINEAR;
    	MagFilter = LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

sampler2D QuadMirrorImageSampler = sampler_state
{
	Texture = (texture0);
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = 2;
	AddressV = 2;	
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

half4 ps_filter(vs_out IN) : COLOR
{
	half4 colorig = tex2D(ImageSampler, IN.texcoord0);			// Texture Screen
	
	
	half2 texcoordquad = IN.texcoord0;

	if ( texcoordquad.x < flipX )								// Mirror X
	{
		texcoordquad.x = 2*flipX-texcoordquad.x;
		if( texcoordquad.x > 1.f ) { texcoordquad.x-=1.f; }
	}
	
	if ( texcoordquad.y < flipY )								// Mirror Y
	{
		texcoordquad.y = 2*flipY-texcoordquad.y;
		if( texcoordquad.y > 1.f ) { texcoordquad.y-=1.f; }
	}


	half precquad = round(lerp(1.0, 5.0, quad));				// Precalc Quad (from 1*1 to 5*5)
	half4 colquad = tex2D(ImageSampler, texcoordquad*precquad);	// Texture "Quad/Mirror"


	if ( quadmirror > 0.5 )
		{ colquad = tex2D(QuadMirrorImageSampler, texcoordquad*precquad); }	// Texture "Auto-Mirror" 


	half4 colorfinal = power*colquad + color*(1.0-power)*colorig;
	half colorigprec = (colorig.r+colorig.g+colorig.b)/3.0f;

	half lumakeyc = lumakey-0.5;

	if ( lumakeyc > 0.05 )
	{
		if ( colorigprec < lumakeyc ) { colorfinal = power*colquad+(1.0-power)*colorig; }
		if ( colorigprec > lumakeyc ) { colorfinal = color*colorig; }
	}

	if ( lumakeyc < -0.05 )
	{
		if ( colorigprec > lumakeyc+1 ) { colorfinal = power*colquad+(1.0-power)*colorig; }
		if ( colorigprec < lumakeyc+1 ) { colorfinal = color*colorig; }
	}

	return colorfinal;

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
