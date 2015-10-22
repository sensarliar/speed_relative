//#include "gps_nmea.h"
//#include "BGS定位授时设备Dlg.h"
#include "calc_rel_pos.h"
#include <math.h>
//#include"stdafx.h"


struct GpsState gps;
/** process noise (is the same for x and y)*/
#define Q       0.1
#define Qdotdot 0.1

/** initial covariance diagonal */
#define INIT_PXX 1.

//TODO: proper measurement noise
#ifndef HFF_R_POS
#define HFF_R_POS   0.01
#endif
#ifndef HFF_R_POS_MIN
#define HFF_R_POS_MIN 0.0001
#endif

#ifndef HFF_R_SPEED
#define HFF_R_SPEED 0.01
#endif
#ifndef HFF_R_SPEED_MIN
#define HFF_R_SPEED_MIN 0.0009
#endif
////20Hz update
#define DT_HFILTER 0.05



/* gps measurement noise */
double Rgps_pos, Rgps_vel;

/*

  X_x = [ x xdot]
  X_y = [ y ydot]


*/
/* output filter states */
struct Hfilterdouble b2_hff_state;


/* these value should be measured by the plane employee
(jiayou?? plane ant as original,jiayou plane head as oridinator x,vertical as z,right hand ordinator)
the position value of tail to the ant at jiayou plane.
may jiayou plane was not coordinate with shouyou plane ??
if know the heading,atitude of jiayou plane and heading of shouyou plane, the const may can be calibrated.

#define POS_TAIL2ANT_JIAYOU_X 0
#define POS_TAIL2ANT_JIAYOU_Y 0
#define POS_TAIL2ANT_JIAYOU_Z 0
*/
double POS_TAIL2ANT_JIAYOU_X=0;
double POS_TAIL2ANT_JIAYOU_Y=0;
double POS_TAIL2ANT_JIAYOU_Z=0;

/* these value should be measured by the plane employee
(shouyou plane ant as original,shouyou plane head as oridinator x,vertical as z,right hand ordinator)
the position value of head to the ant at shouyou plane.

#define POS_HEAD2ANT_SHOUYOU_X 0
#define POS_HEAD2ANT_SHOUYOU_Y 0
#define POS_HEAD2ANT_SHOUYOU_Z 0
*/

double POS_HEAD2ANT_SHOUYOU_X=0;
double POS_HEAD2ANT_SHOUYOU_Y=0;
double POS_HEAD2ANT_SHOUYOU_Z=0;

/* rotate the enu cordinator to the xyz cordinator(shouyou plane ant as original,shouyou plane head as oridinator x,vertical as z,right hand ordinator)
 *should consider the latancy of bestvela message.
*/
/*
void calc_xyz_plane_ordinator()
{
    double rot_angle = 90+gps.speed_angle;//if>360??
    gps.rel_ant2plane_pos.x = gps.rel_ant_pos.x * cos(rot_angle) + gps.rel_ant_pos.y * sin(rot_angle);
    gps.rel_ant2plane_pos.y = gps.rel_ant_pos.y * cos(rot_angle) - gps.rel_ant_pos.x * sin(rot_angle);
    gps.rel_ant2plane_pos.z = gps.rel_ant_pos.z;

}
*/

void calc_enu2xyz_plane_ordinator(struct point_3d *dest,struct point_3d *src)
{
	const double pi=4.0*atan(1.0);
    double rot_angle = (90-gps.speed_angle)*pi/180;//if>360??
//	double test_cos = cos(rot_angle);
//	double test_sin = sin(rot_angle);
    (*dest).x = (*src).x * cos(rot_angle) + (*src).y * sin(rot_angle);
   (*dest).y = (*src).y * cos(rot_angle) - (*src).x * sin(rot_angle);
    (*dest).z = (*src).z;

}


/*the odinator was not changed.
*/
void calc_tail2plane_pos()
{
	double POS_TAIL2ANT_JIAYOU_X_m = 0;
	double POS_TAIL2ANT_JIAYOU_Y_m = 0;
	double POS_TAIL2ANT_JIAYOU_Z_m = 0;
	double speed_angle_jy_rad = 0;
	const double pi=4.0*atan(1.0);
	double rot_angle2 =0;
	if(gps.speed_3d_jy.y > 0)
	{
	speed_angle_jy_rad = atan(gps.speed_3d_jy.x/gps.speed_3d_jy.y);
	rot_angle2 = speed_angle_jy_rad - (gps.speed_angle*pi/180);
	}
	else if(gps.speed_3d_jy.y < 0)
	{
	speed_angle_jy_rad = atan(gps.speed_3d_jy.x/gps.speed_3d_jy.y);
	rot_angle2 = pi + speed_angle_jy_rad - (gps.speed_angle*pi/180);
	}
	else
	{
		rot_angle2 =0;
	}

	POS_TAIL2ANT_JIAYOU_X_m = POS_TAIL2ANT_JIAYOU_X * cos(rot_angle2) + POS_TAIL2ANT_JIAYOU_Y * sin(rot_angle2);
    POS_TAIL2ANT_JIAYOU_Y_m = POS_TAIL2ANT_JIAYOU_Y * cos(rot_angle2) - POS_TAIL2ANT_JIAYOU_X * sin(rot_angle2);
    POS_TAIL2ANT_JIAYOU_Z_m = POS_TAIL2ANT_JIAYOU_Z;


    gps.rel_tail2plane_pos.x = gps.rel_ant2plane_pos.x + POS_TAIL2ANT_JIAYOU_X_m;
    gps.rel_tail2plane_pos.y = gps.rel_ant2plane_pos.y + POS_TAIL2ANT_JIAYOU_Y_m;
    gps.rel_tail2plane_pos.z = gps.rel_ant2plane_pos.z + POS_TAIL2ANT_JIAYOU_Z_m;
}

/*the ordinator was moved to the head of shouyou plane.
*/
void calc_tail2head_pos()
{
        gps.rel_tail2head_pos.x = gps.rel_tail2plane_pos.x - POS_HEAD2ANT_SHOUYOU_X;
        gps.rel_tail2head_pos.y = gps.rel_tail2plane_pos.y - POS_HEAD2ANT_SHOUYOU_Y;
        gps.rel_tail2head_pos.z = gps.rel_tail2plane_pos.z - POS_HEAD2ANT_SHOUYOU_Z;

		
}



/*calculate the relative speed method 1:
 *distance/time = speed ,
 *then convert from enu to xyz plane.
*/

void calc_rel_speed_method1()
{
    gps.rel_speed_enu.x = (gps.rel_ant_pos.x - gps.rel_ant_last_pos.x)/0.05;
    gps.rel_speed_enu.y = (gps.rel_ant_pos.y - gps.rel_ant_last_pos.y)/0.05;
    gps.rel_speed_enu.z = (gps.rel_ant_pos.z - gps.rel_ant_last_pos.z)/0.05;

    calc_enu2xyz_plane_ordinator(&(gps.rel_speed_xyz),&(gps.rel_speed_enu));
}

void calc_rel_speed_method2()
{
	gps.rel_speedv2_enu.x = gps.speed_3d_jy.x - gps.speed_3d_sy.x;
	gps.rel_speedv2_enu.y = gps.speed_3d_jy.y - gps.speed_3d_sy.y;
	gps.rel_speedv2_enu.z = gps.speed_3d_jy.z - gps.speed_3d_sy.z;

	calc_enu2xyz_plane_ordinator(&(gps.rel_speedv2_xyz),&(gps.rel_speedv2_enu));

}



///////////////////////////////////////////



static void b2_hff_init_x(double init_x, double init_xdot)
{
  b2_hff_state.x     = init_x;
  b2_hff_state.xdot  = init_xdot;
  int i, j;
  for (i = 0; i < HFF_STATE_SIZE; i++) {
    for (j = 0; j < HFF_STATE_SIZE; j++) {
      b2_hff_state.xP[i][j] = 0.;
    }
    b2_hff_state.xP[i][i] = INIT_PXX;
  }
}

static void b2_hff_init_y(double init_y, double init_ydot)
{
  b2_hff_state.y     = init_y;
  b2_hff_state.ydot  = init_ydot;
  int i, j;
  for (i = 0; i < HFF_STATE_SIZE; i++) {
    for (j = 0; j < HFF_STATE_SIZE; j++) {
      b2_hff_state.yP[i][j] = 0.;
    }
    b2_hff_state.yP[i][i] = INIT_PXX;
  }
}

void b2_hff_init(double init_x, double init_xdot, double init_y, double init_ydot)
{
  Rgps_pos = HFF_R_POS;
  Rgps_vel = HFF_R_SPEED;
  b2_hff_init_x(init_x, init_xdot);
  b2_hff_init_y(init_y, init_ydot);

}

/*
 *
 * Propagation
 *
 *

 F = [ 1 dt
 0  1 ];

 B = [ dt^2/2 dt]';

 Q = [ 0.01  0
 0     0.01];

 Xk1 = F * Xk0 + B * accel;

 Pk1 = F * Pk0 * F' + Q;

*/
static void b2_hff_propagate_x(struct Hfilterdouble *hff_work, double dt)
{
  /* update state */
  //hff_work->xdotdot = b2_hff_xdd_meas;
  hff_work->x = hff_work->x + dt * hff_work->xdot;
  hff_work->xdot = hff_work->xdot;
  /* update covariance */
  const double FPF00 = hff_work->xP[0][0] + dt * (hff_work->xP[1][0] + hff_work->xP[0][1] + dt * hff_work->xP[1][1]);
  const double FPF01 = hff_work->xP[0][1] + dt * hff_work->xP[1][1];
  const double FPF10 = hff_work->xP[1][0] + dt * hff_work->xP[1][1];
  const double FPF11 = hff_work->xP[1][1];

  hff_work->xP[0][0] = FPF00 + Q;
  hff_work->xP[0][1] = FPF01;
  hff_work->xP[1][0] = FPF10;
  hff_work->xP[1][1] = FPF11 + Qdotdot;
}

static void b2_hff_propagate_y(struct Hfilterdouble *hff_work, double dt)
{
  /* update state */
 // hff_work->ydotdot = b2_hff_ydd_meas;
  hff_work->y = hff_work->y + dt * hff_work->ydot;
  hff_work->ydot = hff_work->ydot;
  /* update covariance */
  const double FPF00 = hff_work->yP[0][0] + dt * (hff_work->yP[1][0] + hff_work->yP[0][1] + dt * hff_work->yP[1][1]);
  const double FPF01 = hff_work->yP[0][1] + dt * hff_work->yP[1][1];
  const double FPF10 = hff_work->yP[1][0] + dt * hff_work->yP[1][1];
  const double FPF11 = hff_work->yP[1][1];

  hff_work->yP[0][0] = FPF00 + Q;
  hff_work->yP[0][1] = FPF01;
  hff_work->yP[1][0] = FPF10;
  hff_work->yP[1][1] = FPF11 + Qdotdot;
}


/*
 *
 * Update position
 *
 *

 H = [1 0];
 R = 0.1;
 // state residual
 y = pos_measurement - H * Xm;
 // covariance residual
 S = H*Pm*H' + R;
 // kalman gain
 K = Pm*H'*inv(S);
 // update state
 Xp = Xm + K*y;
 // update covariance
 Pp = Pm - K*H*Pm;
*/

static void b2_hff_update_x(struct Hfilterdouble *hff_work, double x_meas, double Rpos)
{
  //b2_hff_x_meas = x_meas;

  const double y  = x_meas - hff_work->x;
  const double S  = hff_work->xP[0][0] + Rpos;
  const double K1 = hff_work->xP[0][0] * 1 / S;
  const double K2 = hff_work->xP[1][0] * 1 / S;

  hff_work->x     = hff_work->x     + K1 * y;
  hff_work->xdot  = hff_work->xdot  + K2 * y;

  const double P11 = (1. - K1) * hff_work->xP[0][0];
  const double P12 = (1. - K1) * hff_work->xP[0][1];
  const double P21 = -K2 * hff_work->xP[0][0] + hff_work->xP[1][0];
  const double P22 = -K2 * hff_work->xP[0][1] + hff_work->xP[1][1];

  hff_work->xP[0][0] = P11;
  hff_work->xP[0][1] = P12;
  hff_work->xP[1][0] = P21;
  hff_work->xP[1][1] = P22;
}

static void b2_hff_update_y(struct Hfilterdouble *hff_work, double y_meas, double Rpos)
{
 // b2_hff_y_meas = y_meas;

  const double y  = y_meas - hff_work->y;
  const double S  = hff_work->yP[0][0] + Rpos;
  const double K1 = hff_work->yP[0][0] * 1 / S;
  const double K2 = hff_work->yP[1][0] * 1 / S;

  hff_work->y     = hff_work->y     + K1 * y;
  hff_work->ydot  = hff_work->ydot  + K2 * y;

  const double P11 = (1. - K1) * hff_work->yP[0][0];
  const double P12 = (1. - K1) * hff_work->yP[0][1];
  const double P21 = -K2 * hff_work->yP[0][0] + hff_work->yP[1][0];
  const double P22 = -K2 * hff_work->yP[0][1] + hff_work->yP[1][1];

  hff_work->yP[0][0] = P11;
  hff_work->yP[0][1] = P12;
  hff_work->yP[1][0] = P21;
  hff_work->yP[1][1] = P22;
}


/*
 *
 * Update velocity
 *
 *

 H = [0 1];
 R = 0.1;
 // state residual
 yd = vx - H * Xm;
 // covariance residual
 S = H*Pm*H' + R;
 // kalman gain
 K = Pm*H'*inv(S);
 // update state
 Xp = Xm + K*yd;
 // update covariance
 Pp = Pm - K*H*Pm;
*/


static void b2_hff_update_xdot(struct Hfilterdouble *hff_work, double vel, double Rvel)
{
  //b2_hff_xd_meas = vel;

  const double yd = vel - hff_work->xdot;
  const double S  = hff_work->xP[1][1] + Rvel;
  const double K1 = hff_work->xP[0][1] * 1 / S;
  const double K2 = hff_work->xP[1][1] * 1 / S;

  hff_work->x     = hff_work->x     + K1 * yd;
  hff_work->xdot  = hff_work->xdot  + K2 * yd;

  const double P11 = -K1 * hff_work->xP[1][0] + hff_work->xP[0][0];
  const double P12 = -K1 * hff_work->xP[1][1] + hff_work->xP[0][1];
  const double P21 = (1. - K2) * hff_work->xP[1][0];
  const double P22 = (1. - K2) * hff_work->xP[1][1];

  hff_work->xP[0][0] = P11;
  hff_work->xP[0][1] = P12;
  hff_work->xP[1][0] = P21;
  hff_work->xP[1][1] = P22;
}

static void b2_hff_update_ydot(struct Hfilterdouble *hff_work, double vel, double Rvel)
{
//  b2_hff_yd_meas = vel;

  const double yd = vel - hff_work->ydot;
  const double S  = hff_work->yP[1][1] + Rvel;
  const double K1 = hff_work->yP[0][1] * 1 / S;
  const double K2 = hff_work->yP[1][1] * 1 / S;

  hff_work->y     = hff_work->y     + K1 * yd;
  hff_work->ydot  = hff_work->ydot  + K2 * yd;

  const double P11 = -K1 * hff_work->yP[1][0] + hff_work->yP[0][0];
  const double P12 = -K1 * hff_work->yP[1][1] + hff_work->yP[0][1];
  const double P21 = (1. - K2) * hff_work->yP[1][0];
  const double P22 = (1. - K2) * hff_work->yP[1][1];

  hff_work->yP[0][0] = P11;
  hff_work->yP[0][1] = P12;
  hff_work->yP[1][0] = P21;
  hff_work->yP[1][1] = P22;
}

//void b2_hff_update_gps(struct FloatVect2 *pos_ned, struct FloatVect2 *speed_ned)
void b2_hff_update_gps()
{

#if USE_GPS_ACC4R
  Rgps_pos = (double) gps.pacc / 100.;
  if (Rgps_pos < HFF_R_POS_MIN) {
    Rgps_pos = HFF_R_POS_MIN;
  }

  Rgps_vel = (double) gps.sacc / 100.;
  if (Rgps_vel < HFF_R_SPEED_MIN) {
    Rgps_vel = HFF_R_SPEED_MIN;
  }
#endif


    /* update filter state with measurement */
    b2_hff_update_x(&b2_hff_state, gps.rel_ant_pos_enu_measure.x, Rgps_pos);
    b2_hff_update_y(&b2_hff_state, gps.rel_ant_pos_enu_measure.y, Rgps_pos);

    b2_hff_update_xdot(&b2_hff_state, gps.rel_speedv2_enu_measure.x, Rgps_vel);
    b2_hff_update_ydot(&b2_hff_state, gps.rel_speedv2_enu_measure.y, Rgps_vel);

  }

void b2_hff_propagate(void)
{
//// output result befor propagate;,
	b2_hff_propagate_x(&b2_hff_state, DT_HFILTER);
    b2_hff_propagate_y(&b2_hff_state, DT_HFILTER);
}