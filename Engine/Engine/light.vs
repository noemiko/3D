#define NUM_LIGHTS 4

cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer LightPositionBuffer
{
    float4 lightPosition[NUM_LIGHTS];
};

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
	float3 lightPos1 : TEXCOORD1;
    float3 lightPos2 : TEXCOORD2;
    float3 lightPos3 : TEXCOORD3;
    float3 lightPos4 : TEXCOORD4;
};


PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;

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

	// Oblicz pozycjê vertexa w œwiecie
    worldPosition = mul(input.position, worldMatrix);


	//Okreœl po³o¿enie œwiat³a na podstawie po³o¿enia œwiate³ i po³o¿enia vertexa na œwiecie.    
	output.lightPos1.xyz = lightPosition[0].xyz - worldPosition.xyz;
    output.lightPos2.xyz = lightPosition[1].xyz - worldPosition.xyz;
    output.lightPos3.xyz = lightPosition[2].xyz - worldPosition.xyz;
    output.lightPos4.xyz = lightPosition[3].xyz - worldPosition.xyz;

    // Normalizuj pozycjê wektorów œwiate³
    output.lightPos1 = normalize(output.lightPos1);
    output.lightPos2 = normalize(output.lightPos2);
    output.lightPos3 = normalize(output.lightPos3);
    output.lightPos4 = normalize(output.lightPos4);

    return output;
}