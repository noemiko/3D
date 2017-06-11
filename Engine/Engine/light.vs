/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};


PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    

	// Zmieniæ wektor pozycji na 4 jednostki w celu uzyskania prawid³owych obliczeñ tablicy.
    input.position.w = 1.0f;

	// Obliczyæ pozycjê vertexa wzglêdem œwiatów, widoków i macierzy projekcyjnych.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Zapisz koordynaty textury w pixel shaderze.
	output.tex = input.tex;
    
	// Obliczyæ normalny wektor tylko wzglêdem macierzy œwiatowej.
    output.normal = mul(input.normal, (float3x3)worldMatrix);
	
    // Normalizuj normal vector.
    output.normal = normalize(output.normal);

    return output;
}