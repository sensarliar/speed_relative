#ifndef CALC_REL_POS_H
#define CALC_REL_POS_H

struct point_3d {
    double x;
    double y;
    double z;
};

struct GpsState {
  double speed_angle;

  struct point_3d rel_ant_last_pos;

  struct point_3d rel_ant_pos;// jiayou plane ant pos (shouyou plane ant as original,enu)

  struct point_3d rel_ant2plane_pos;// jiayou plane ant pos (shouyou plane ant as original,shouyou plane head as oridinator x,vertical as z,right hand ordinator)

  struct point_3d rel_tail2plane_pos;// jiayou plane tail pos(shouyou plane ant as original,shouyou plane head as oridinator x,vertical as z,right hand ordinator)

  struct point_3d rel_tail2head_pos;// jiayou plane tail pos(shouyou plane head as original,shouyou plane head as oridinator x,vertical as z,right hand ordinator)


  struct point_3d rel_speed_enu;
  struct point_3d rel_speed_xyz;

  struct point_3d speed_3d_sy;
  struct point_3d speed_3d_jy;

  struct point_3d rel_speedv2_enu;
  struct point_3d rel_speedv2_xyz;
};


extern struct GpsState gps;


//void calc_xyz_plane_ordinator();
void calc_enu2xyz_plane_ordinator(struct point_3d *dest,struct point_3d *src);

void calc_tail2plane_pos();


/*the ordinator was moved to the head of shouyou plane.
*/
void calc_tail2head_pos();

void calc_rel_speed_method1();
void calc_rel_speed_method2();


#endif // CALC_REL_POS_H
