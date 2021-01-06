#pragma once
#include <d3d11.h>
#include "objfilemodel.h"

class Model
{
protected:

	struct MODEL_CONSTANT_BUFFER
	{
		XMMATRIX WorldViewProjection;
		XMMATRIX WorldView;
		XMVECTOR directional_light_vector;
		XMVECTOR directional_light_colour;
		XMVECTOR ambient_light_vector;
	};

	MODEL_CONSTANT_BUFFER model_cb_values;

	ID3D11Device* m_pD3DDevice;
	ID3D11DeviceContext* m_pImmediateContext;

	ObjFileModel* m_pObject;
	ID3D11VertexShader* m_pVShader;
	ID3D11PixelShader* m_pPShader;
	ID3D11GeometryShader* m_pGShader;
	ID3D11InputLayout* m_pInputLayout;
	ID3D11Buffer* m_pConstantBuffer;

	ID3D11ShaderResourceView* m_pTexture0;
	ID3D11SamplerState* m_pSampler0;

	XMMATRIX world;

	XMMATRIX transpose;
	XMVECTOR directionalLightColor;
	XMVECTOR ambientLightColor; 
	XMVECTOR directionalLightVector;

	float m_x, m_y, m_z;
	float m_dx, m_dy, m_dz;
	float m_xAngle, m_yAngle, m_zAngle;
	float m_xScale, m_yScale, m_zScale;
	float m_bounding_sphere_centre_x, m_bounding_sphere_centre_y, m_bounding_sphere_centre_z;
	float m_bounding_sphere_radius;
	float maxBoundsX, minBoundsX, maxBoundsY, minBoundsY, maxBoundsZ, minBoundsZ;

	void CalculateModelCentrePoint();
	void CalculateBoundingSphereRadius();
	XMVECTOR GetBoundingSphereWorldSpacePosition();
	float GetBoundingSphereRadius();

public:

	Model(ID3D11Device* device, ID3D11DeviceContext* context);
	~Model();

	HRESULT InitModel();
	HRESULT LoadObjModel(char* filename);
	HRESULT UseShader(char* mainShader, char* vertexShaderName, char* pixelShaderName);
	HRESULT AddTexture(char* filename);
	void CalculateLighting(XMVECTOR directionalLightColor, XMVECTOR ambientLightColor, XMVECTOR directionalLightVector);
	void Draw(XMMATRIX* view, XMMATRIX* projection);

	void CalculateCollisionData();
	bool CheckSphereToBoxCollision(Model* modelBox);
	bool CheckCollision(Model* model);
	void CheckCollisionAll(vector<Model*> models);

	void LookAt_XZ(float objX, float objZ);
	void LookAt_XZ_Reversed(float objX, float objZ);
	void MoveForward(float distance);

	XMFLOAT3 GetPosition();

	float GetDX();
	float GetDZ();
	void IncPos(XMFLOAT3 newPos);
	void IncPos(float x, float y, float z);
	void SetPos(XMFLOAT3 newPos);
	void SetPos(float x, float y, float z);
	void SetPosX(float x);
	void SetPosY(float y);
	void SetPosZ(float z);
	float GetPosX();
	float GetPosY();
	float GetPosZ();
	void IncRot(float x, float y, float z);
	void SetRot(float x, float y, float z);
	void SetRotX(float x);
	void SetRotY(float y);
	void SetRotZ(float z);
	float GetRotX();
	float GetRotY();
	float GetRotZ();
	void IncScale(float x, float y, float z);
	void SetScale(float x, float y, float z);
	void SetScaleX(float x);
	void SetScaleY(float y);
	void SetScaleZ(float z);
	float GetScaleX();
	float GetScaleY();
	float GetScaleZ();
};