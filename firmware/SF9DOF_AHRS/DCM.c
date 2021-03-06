#include "calculations.h"
#include "math.h"
#include "vector.h"
#include "stdint.h"

static float Accel_Vector[3] =
{ 0, 0, 0 }; //Store the acceleration in a vector
static float Gyro_Vector[3] =
{ 0, 0, 0 };//Store the gyros turn rate in a vector
static float Omega_Vector[3] =
{ 0, 0, 0 }; //Corrected Gyro_Vector data
static float Omega_P[3] =
{ 0, 0, 0 };//Omega Proportional correction
static float Omega_I[3] =
{ 0, 0, 0 };//Omega Integrator
static float Omega[3] =
{ 0, 0, 0 };

static float errorRollPitch[3] =
{ 0, 0, 0 };
static float errorYaw[3] =
{ 0, 0, 0 };

static float DCM_Matrix[3][3] =
{
{ 1, 0, 0 },
{ 0, 1, 0 },
{ 0, 0, 1 } };
static float Update_Matrix[3][3] =
{
{ 0, 1, 2 },
{ 3, 4, 5 },
{ 6, 7, 8 } }; //Gyros here

static float Temporary_Matrix[3][3] =
{
{ 0, 0, 0 },
{ 0, 0, 0 },
{ 0, 0, 0 } };

static float G_Dt = 0.02; // Integration time (DCM algorithm)  We will run the integration loop at 50Hz if possible

/******************Gyro / Acc. updates ********************/

// parameter is pointer to array with 3xAccelerometer data in x/y/z format
void updateAccelero(uint16_t *values)
{
	accel_x = *values;
	accel_y = *(values + 1);
	accel_z = *(values + 2);
}

/*******************Matrix.c		************************/
static void Matrix_Multiply(float *a, float *b, float* mat)
{
	int x, y, w;
	float op[3];

	for (x = 0; x < 3; x++)
	{
		for (y = 0; y < 3; y++)
		{
			for (w = 0; w < 3; w++)
			{
				op[w] = (*(a + ((x * 3) + w))) * (*(b + ((x * 3) + w)));
			}

			*(mat + ((x * 3) + y)) = 0;
			*(mat + ((x * 3) + y)) = op[0] + op[1] + op[2];

			//float test = mat[x][y];
		}
	}

}
/*******************End Matrix.c		********************/

void dcmElapsedTime(uint16_t timeSinceLastRun)
{
	//G_Dt = (timer - timer_old) / 1000.0; // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
	/*		// timer zal altijd groter zijn tenzij bij init. 1000 == mS
	 if (timer > timer_old)
	 G_Dt = (timer - timer_old) / 1000.0; // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
	 else
	 G_Dt = 0;*/
}
/**************************************************/
void Normalize(void)
{
	float error = 0;
	float temporary[3][3];
	float renorm = 0;

	error = -Vector_Dot_Product(&DCM_Matrix[0][0], &DCM_Matrix[1][0]) * .5; //eq.19

	Vector_Scale(&temporary[0][0], &DCM_Matrix[1][0], error); //eq.19
	Vector_Scale(&temporary[1][0], &DCM_Matrix[0][0], error); //eq.19

	Vector_Add(&temporary[0][0], &temporary[0][0], &DCM_Matrix[0][0]);//eq.19
	Vector_Add(&temporary[1][0], &temporary[1][0], &DCM_Matrix[1][0]);//eq.19

	Vector_Cross_Product(&temporary[2][0], &temporary[0][0], &temporary[1][0]); // c= a x b //eq.20

	renorm = .5 * (3 - Vector_Dot_Product(&temporary[0][0], &temporary[0][0])); //eq.21
	Vector_Scale(&DCM_Matrix[0][0], &temporary[0][0], renorm);

	renorm = .5 * (3 - Vector_Dot_Product(&temporary[1][0], &temporary[1][0])); //eq.21
	Vector_Scale(&DCM_Matrix[1][0], &temporary[1][0], renorm);

	renorm = .5 * (3 - Vector_Dot_Product(&temporary[2][0], &temporary[2][0])); //eq.21
	Vector_Scale(&DCM_Matrix[2][0], &temporary[2][0], renorm);
}

/**************************************************/
void Drift_correction(void)
{
	//Compensation the Roll, Pitch and Yaw drift.
	static float Scaled_Omega_I[3];
	float Accel_magnitude;
	float Accel_weight;

	//*****Roll and Pitch***************

	// Calculate the magnitude of the accelerometer vector
	Accel_magnitude = sqrt(Accel_Vector[0] * Accel_Vector[0] + Accel_Vector[1]
			* Accel_Vector[1] + Accel_Vector[2] * Accel_Vector[2]);
	Accel_magnitude = Accel_magnitude / GRAVITY_DIV; // Scale to gravity.
	// Dynamic weighting of accelerometer info (reliability filter)
	// Weight for accelerometer info (<0.5G = 0.0, 1G = 1.0 , >1.5G = 0.0)
	Accel_weight = constrain(1 - 2 * abs(1 - Accel_magnitude), 0, 1); //

	Vector_Cross_Product(&errorRollPitch[0], &Accel_Vector[0],
			&DCM_Matrix[2][0]); //adjust the ground of reference
	Vector_Scale(&Omega_P[0], &errorRollPitch[0], Kp_ROLLPITCH * Accel_weight);

	Vector_Scale(&Scaled_Omega_I[0], &errorRollPitch[0], Ki_ROLLPITCH
			* Accel_weight);
	Vector_Add(Omega_I, Omega_I, Scaled_Omega_I);

#ifdef NO_MAGNETOMETER
	float mag_heading_x;
	float mag_heading_y;
	float errorCourse;
	static float Scaled_Omega_P[3];
	//*****YAW***************
	// We make the gyro YAW drift correction based on compass magnetic heading

	mag_heading_x = cos(MAG_Heading);
	mag_heading_y = sin(MAG_Heading);
	errorCourse = (DCM_Matrix[0][0] * mag_heading_y) - (DCM_Matrix[1][0]
			* mag_heading_x); //Calculating YAW error
	Vector_Scale(errorYaw, &DCM_Matrix[2][0], errorCourse); //Applys the yaw correction to the XYZ rotation of the aircraft, depeding the position.

	Vector_Scale(&Scaled_Omega_P[0], &errorYaw[0], Kp_YAW);//.01proportional of YAW.
	Vector_Add(Omega_P, Omega_P, Scaled_Omega_P);//Adding  Proportional.

	Vector_Scale(&Scaled_Omega_I[0], &errorYaw[0], Ki_YAW);//.00001Integrator
	Vector_Add(Omega_I, Omega_I, Scaled_Omega_I);//adding integrator to the Omega_I
#endif
}
/**************************************************/
/*
 void Accel_adjust(void)
 {
 Accel_Vector[1] += Accel_Scale(speed_3d*Omega[2]);  // Centrifugal force on Acc_y = GPS_speed*GyroZ
 Accel_Vector[2] -= Accel_Scale(speed_3d*Omega[1]);  // Centrifugal force on Acc_z = GPS_speed*GyroY 
 }
 */
/**************************************************/
typedef struct
{
	int accel_x; // gecompenseerd met off-set
	int accel_y;// gecompenseerd met off-set
	int accel_z;// gecompenseerd met off-set
	float gyro_x;// raw data of the gyro in radians per second, use:
	float gyro_y;//  #define ToRad(x) (x*0.01745329252)  // *pi/180
	float gyro_z;// for this calculation
} MATRIX_UPDATE_STRUCT;

void Matrix_update(MATRIX_UPDATE_STRUCT *p)
{
	Gyro_Vector[0] = p->gyro_x;//Gyro_Scaled_X(read_adc(0)); //gyro x roll
	Gyro_Vector[1] = p->gyro_y;//Gyro_Scaled_Y(read_adc(1)); //gyro y pitch
	Gyro_Vector[2] = p->gyro_z;//Gyro_Scaled_Z(read_adc(2)); //gyro Z yaw

	Accel_Vector[0] = p->accel_x;
	Accel_Vector[1] = p->accel_y;
	Accel_Vector[2] = p->accel_z;

	Vector_Add(&Omega[0], &Gyro_Vector[0], &Omega_I[0]); //adding proportional term
	Vector_Add(&Omega_Vector[0], &Omega[0], &Omega_P[0]); //adding Integrator term

	//Accel_adjust();    //Remove centrifugal acceleration.   We are not using this function in this version - we have no speed measurement

#if OUTPUTMODE==1
	Update_Matrix[0][0] = 0;
	Update_Matrix[0][1] = -G_Dt * Omega_Vector[2];//-z
	Update_Matrix[0][2] = G_Dt * Omega_Vector[1];//y
	Update_Matrix[1][0] = G_Dt * Omega_Vector[2];//z
	Update_Matrix[1][1] = 0;
	Update_Matrix[1][2] = -G_Dt * Omega_Vector[0];//-x
	Update_Matrix[2][0] = -G_Dt * Omega_Vector[1];//-y
	Update_Matrix[2][1] = G_Dt * Omega_Vector[0];//x
	Update_Matrix[2][2] = 0;
#else                    // Uncorrected data (no drift correction)
	Update_Matrix[0][0] = 0;
	Update_Matrix[0][1] = -G_Dt * Gyro_Vector[2];//-z
	Update_Matrix[0][2] = G_Dt * Gyro_Vector[1];//y
	Update_Matrix[1][0] = G_Dt * Gyro_Vector[2];//z
	Update_Matrix[1][1] = 0;
	Update_Matrix[1][2] = -G_Dt * Gyro_Vector[0];
	Update_Matrix[2][0] = -G_Dt * Gyro_Vector[1];
	Update_Matrix[2][1] = G_Dt * Gyro_Vector[0];
	Update_Matrix[2][2] = 0;
#endif

	Matrix_Multiply(DCM_Matrix, Update_Matrix, Temporary_Matrix); //a*b=c

	int x, y;
	for (x = 0; x < 3; x++) //Matrix Addition (update)
	{
		for (y = 0; y < 3; y++)
		{
			DCM_Matrix[x][y] += Temporary_Matrix[x][y];
		}
	}
}

// Euler angles
GYRO_STRUCT gyro;

void Euler_angles(void)
{
	gyro.pitch = -asin(DCM_Matrix[2][0]);
	gyro.roll = atan2(DCM_Matrix[2][1], DCM_Matrix[2][2]);
	gyro.yaw = atan2(DCM_Matrix[1][0], DCM_Matrix[0][0]);
}

