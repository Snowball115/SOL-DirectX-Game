#pragma once

#include "Model.h"
#include "Camera.h"

class MovableObject : public Model
{
private:

public:

	MovableObject(ID3D11Device* device, ID3D11DeviceContext* context);
	~MovableObject();

	void Move(Camera* pusher);
};

