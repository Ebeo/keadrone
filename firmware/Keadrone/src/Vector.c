

//Computes the dot product of two vectors
float Vector_Dot_Product(float *vector1, float *vector2)
{
	float op = 0;
	int c;
	for (c = 0; c < 3; c++)
	{
		op += *(vector1 + c) * (*(vector2 + c));
	}

	return op;
}

//Computes the cross product of two vectors
void Vector_Cross_Product(float *vectorOut, float *v1, float *v2)
{
	*(vectorOut + 0) = (*(v1 + 1) * *(v2 + 2)) - (*(v1 + 2) * (*(v2 + 1)));
	*(vectorOut + 1) = (*(v1 + 2) * *(v2 + 0)) - (*(v1 + 0) * (*(v2 + 2)));
	*(vectorOut + 2) = (*(v1 + 0) * *(v2 + 1)) - (*(v1 + 1) * (*(v2 + 0)));
}

//Multiply the vector by a scalar. 
void Vector_Scale(float *vectorOut, float *vectorIn, float scale2)
{
	int c;
	for (c = 0; c < 3; c++)
	{
		*(vectorOut + c) = *(vectorIn + c) * scale2;
	}
}

void Vector_Add(float *vectorOut, float *vectorIn1, float *vectorIn2)
{
	int c;
	for (c = 0; c < 3; c++)
	{
		*(vectorOut + c) = *(vectorIn1 + c) + *(vectorIn2 + c);
	}
}

