#include "Model.h"
#include "Utilities.h"

Model::Model(ID3D11Device* device, ID3D11DeviceContext* context) 
{
	m_pD3DDevice = device;
	m_pImmediateContext = context;

	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 0.0f;
	m_xAngle = 0.0f;
	m_yAngle = 0.0f;
	m_zAngle = 0.0f;
	m_xScale = 1.0f;
	m_yScale = 1.0f;
	m_zScale = 1.0f;

	InitModel();
}

Model::~Model()
{
	if (m_pObject) delete m_pObject;
	if (m_pTexture0) m_pTexture0->Release();
	if (m_pSampler0) m_pSampler0->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pVShader) m_pVShader->Release();
	if (m_pPShader) m_pPShader->Release();
}

HRESULT Model::InitModel() 
{
	HRESULT hr;

	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));
	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;					// Can use UpdateSubresource() to update
	constant_buffer_desc.ByteWidth = sizeof(MODEL_CONSTANT_BUFFER);		// MUST be a multiple of 16, calculate from CB struct
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		// Use a constant buffer
	m_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &m_pConstantBuffer);

	//Load and compile the pixel and vertex shaders - use vs_5_0 to target DX11 hardware only
	ID3DBlob* VS, * PS, * error;

	D3DX11CompileFromFile("model_shader.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);
	D3DX11CompileFromFile("model_shader.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);

	//Create shader objects
	m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVShader);
	m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPShader);

	//Set the shader objects as active
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);

	//Create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);

	return S_OK;
}

// Load an obj Model
HRESULT Model::LoadObjModel(char* filename) 
{
	m_pObject = new ObjFileModel(filename, m_pD3DDevice, m_pImmediateContext);

	if (m_pObject->filename == "FILE NOT LOADED") return S_FALSE;

	return S_OK;
}

// Apply shader on model
HRESULT Model::UseShader(char* mainShader, char* vertexShaderName, char* pixelShaderName)
{
	HRESULT hr;

	//Load and compile the pixel and vertex shaders - use vs_5_0 to target DX11 hardware only
	ID3DBlob* VS, * PS, * error;

	hr = D3DX11CompileFromFile(mainShader, 0, 0, vertexShaderName, "vs_4_0", 0, 0, 0, &VS, &error, 0);
	if (FAILED(hr)) D3DX11CompileFromFile("model_shader.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);

	hr = D3DX11CompileFromFile(mainShader, 0, 0, pixelShaderName, "ps_4_0", 0, 0, 0, &PS, &error, 0);
	if (FAILED(hr)) D3DX11CompileFromFile("model_shader.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);

	//Create shader objects
	m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVShader);
	m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPShader);

	//Set the shader objects as active
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);

	return S_OK;
}

// Apply texture on model
HRESULT Model::AddTexture(char* filename)
{
	// Load texture
	D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice, filename, NULL, NULL, &m_pTexture0, NULL);

	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_pD3DDevice->CreateSamplerState(&sampler_desc, &m_pSampler0);

	return S_OK;
}

// Get current lighting calculations
void Model::CalculateLighting(XMVECTOR directionalLightColor, XMVECTOR ambientLightColor, XMVECTOR directionalLightVector)
{
	this->directionalLightColor = directionalLightColor;
	this->ambientLightColor = ambientLightColor;
	this->directionalLightVector = directionalLightVector;
}

void Model::Draw(XMMATRIX* view, XMMATRIX* projection)
{
	world = XMMatrixScaling(m_xScale, m_yScale, m_zScale);
	world *= XMMatrixRotationX(XMConvertToRadians(m_xAngle));
	world *= XMMatrixRotationY(XMConvertToRadians(m_yAngle));
	world *= XMMatrixRotationZ(XMConvertToRadians(m_zAngle));
	world *= XMMatrixTranslation(m_x, m_y, m_z);

	model_cb_values.WorldViewProjection = world * (*view) * (*projection);
	model_cb_values.WorldView = world * (*view);

	// Update constant buffer
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &model_cb_values, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	// Set as active ones
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	// Use texture
	m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampler0);
	m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTexture0);

	// Calculate lighting
	transpose = XMMatrixTranspose(world);
	model_cb_values.directional_light_colour = directionalLightColor;
	model_cb_values.ambient_light_vector = ambientLightColor;
	model_cb_values.directional_light_vector = XMVector3Transform(directionalLightVector, transpose);
	model_cb_values.directional_light_vector = XMVector3Normalize(model_cb_values.directional_light_vector);

	m_pObject->Draw();
}

void Model::CalculateModelCentrePoint() 
{
	for (int i = 0; i < m_pObject->numverts; i++) 
	{
		if (m_pObject->vertices[i].Pos.x < minBoundsX) minBoundsX = m_pObject->vertices[i].Pos.x;
		if (m_pObject->vertices[i].Pos.x > maxBoundsX) maxBoundsX = m_pObject->vertices[i].Pos.x;

		if (m_pObject->vertices[i].Pos.y < minBoundsY) minBoundsY = m_pObject->vertices[i].Pos.y;
		if (m_pObject->vertices[i].Pos.y > maxBoundsY) maxBoundsY = m_pObject->vertices[i].Pos.y;

		if (m_pObject->vertices[i].Pos.z < minBoundsZ) minBoundsZ = m_pObject->vertices[i].Pos.z;
		if (m_pObject->vertices[i].Pos.z > maxBoundsZ) maxBoundsZ = m_pObject->vertices[i].Pos.z;

	}

	m_bounding_sphere_centre_x = minBoundsX + (maxBoundsX - minBoundsX) / 2;
	m_bounding_sphere_centre_y = minBoundsY + (maxBoundsY - minBoundsY) / 2;
	m_bounding_sphere_centre_z = minBoundsZ + (maxBoundsZ - minBoundsZ) / 2;

	/*Utilities::DebugLog(m_bounding_sphere_centre_x, "x ");
	Utilities::DebugLog(m_bounding_sphere_centre_y, "y ");
	Utilities::DebugLog(m_bounding_sphere_centre_z, "z ");*/
}

void Model::CalculateBoundingSphereRadius() 
{
	float distance = 0;

	for (int i = 0; i < m_pObject->numverts; i++) 
	{
		distance = sqrtf(powf(m_pObject->vertices[i].Pos.x - m_bounding_sphere_centre_x, 2) +
						 powf(m_pObject->vertices[i].Pos.y - m_bounding_sphere_centre_y, 2) +
						 powf(m_pObject->vertices[i].Pos.z - m_bounding_sphere_centre_z, 2));
	}

	m_bounding_sphere_radius = distance;

	//Utilities::DebugLog(m_bounding_sphere_radius, "radius ");
}

XMVECTOR Model::GetBoundingSphereWorldSpacePosition()
{
	XMMATRIX world;
	XMVECTOR offset;

	world = XMMatrixScaling(m_xScale, m_yScale, m_zScale);
	world *= XMMatrixRotationX(XMConvertToRadians(m_xAngle));
	world *= XMMatrixRotationY(XMConvertToRadians(m_yAngle));
	world *= XMMatrixRotationZ(XMConvertToRadians(m_zAngle));
	world *= XMMatrixTranslation(m_x, m_y, m_z);

	offset = XMVectorSet(m_bounding_sphere_centre_x, m_bounding_sphere_centre_y, m_bounding_sphere_centre_z, 0.0f);
	offset = XMVector3Transform(offset, world);

	return offset;
}

float Model::GetBoundingSphereRadius() 
{
	return m_bounding_sphere_radius * m_xScale;
}

void Model::CalculateCollisionData()
{
	CalculateModelCentrePoint();
	CalculateBoundingSphereRadius();
}

// Check from box Sphere to Box collision (for example on walls)
bool Model::CheckSphereToBoxCollision(Model* modelBox)
{
	//modelBox->CalculateModelCentrePoint();

	float x = Utilities::MaxValue(modelBox->minBoundsX, Utilities::MinValue(m_x, modelBox->maxBoundsX));
	float y = Utilities::MaxValue(modelBox->minBoundsY, Utilities::MinValue(m_y, modelBox->maxBoundsY));
	float z = Utilities::MaxValue(modelBox->minBoundsZ, Utilities::MinValue(m_z, modelBox->maxBoundsZ));

	float distance = sqrtf((x - m_x) * (x - m_x) + 
						   (y - m_y) * (y - m_y) +
						   (z - m_z) * (z - m_z));

	//Utilities::DebugLog(m_bounding_sphere_radius);

	if (distance < GetBoundingSphereRadius())
	{
		//Utilities::DebugLog("COLLISION");
		return true;
	}

	return false;
}

bool Model::CheckCollision(Model *model) 
{
	if (model == this) return false;

	XMVECTOR currentModel = GetBoundingSphereWorldSpacePosition();
	XMVECTOR otherModel = model->GetBoundingSphereWorldSpacePosition();

	float x1 = XMVectorGetX(currentModel);
	float x2 = XMVectorGetX(otherModel);
	float y1 = XMVectorGetY(currentModel);
	float y2 = XMVectorGetY(otherModel);
	float z1 = XMVectorGetZ(currentModel);
	float z2 = XMVectorGetZ(otherModel);

	float distance = powf(x1 - x2, 2) + powf(y1 - y2, 2) + powf(z1 - z2, 2);

	if (distance <= powf(GetBoundingSphereRadius() + model->GetBoundingSphereRadius(), 2))
	{
		//Utilities::DebugLog("COLLISION");
		return true;
	}
	
	return false; 
}

void Model::CheckCollisionAll(vector<Model*> models) 
{
	for (int i = 0; i < models.size(); i++) 
	{
		CheckCollision(models[i]);
	}
}

// Face model towards a direction
void Model::LookAt_XZ(float objX, float objZ)
{
	m_dx = objX - m_x;
	m_dz = objZ - m_z;
	m_yAngle = XMConvertToDegrees(atan2(m_dx, m_dz));
}

// Face model away from a direction (Used for running away from player)
void Model::LookAt_XZ_Reversed(float objX, float objZ)
{
	m_dx = objX - m_x;
	m_dz = objZ - m_z;
	m_yAngle = XMConvertToDegrees(atan2(-m_dx, -m_dz));
}

// Move model towards the direction its facing
void Model::MoveForward(float distance) 
{
	m_x += sin(XMConvertToRadians(m_yAngle)) * distance;
	m_z += cos(XMConvertToRadians(m_yAngle)) * distance;
}

XMFLOAT3 Model::GetPosition() 
{
	return XMFLOAT3(m_x, m_y, m_z);
}

float Model::GetDX()
{
	return m_dx;
}

float Model::GetDZ()
{
	return m_dz;
}

// Increasing and setting POSITION
void Model::IncPos(XMFLOAT3 newPos) 
{
	m_x += newPos.x;
	m_y += newPos.y;
	m_z += newPos.z;
}

void Model::IncPos(float x, float y, float z) 
{
	m_x += x;
	m_y += y;
	m_z += z;
}

void Model::SetPos(XMFLOAT3 newPos)
{
	m_x = newPos.x;
	m_y = newPos.y;
	m_z = newPos.z;
}

void Model::SetPos(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

void Model::SetPosX(float x) { m_x = x; }
void Model::SetPosY(float y) { m_y = y; }
void Model::SetPosZ(float z) { m_z = z; }
float Model::GetPosX() { return m_x; }
float Model::GetPosY() { return m_y; }
float Model::GetPosZ() { return m_z; }

// Increasing and setting ROTATION
void Model::IncRot(float x, float y, float z)
{
	m_xAngle += x;
	m_yAngle += y;
	m_zAngle += z;
}

void Model::SetRot(float x, float y, float z)
{
	m_xAngle = x;
	m_yAngle = y;
	m_zAngle = z;
}

void Model::SetRotX(float x) { m_xAngle = x; }
void Model::SetRotY(float y) { m_yAngle = y; }
void Model::SetRotZ(float z) { m_zAngle = z; }
float Model::GetRotX() { return m_xAngle; }
float Model::GetRotY() { return m_yAngle; }
float Model::GetRotZ() { return m_zAngle; }

// Increasing and setting SCALE
void Model::IncScale(float x, float y, float z)
{
	m_xScale += x;
	m_yScale += y;
	m_zScale += z;
}

void Model::SetScale(float x, float y, float z)
{
	m_xScale = x;
	m_yScale = y;
	m_zScale = z;
}

void Model::SetScaleX(float x) { m_xScale = x; }
void Model::SetScaleY(float y) { m_yScale = y; }
void Model::SetScaleZ(float z) { m_zScale = z; }
float Model::GetScaleX() { return m_xScale; }
float Model::GetScaleY() { return m_yScale; }
float Model::GetScaleZ() { return m_zScale; }