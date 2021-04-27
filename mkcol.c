#include<stdio.h>
#define SYSMAX 255

int main(void)
{
  int     ic, n_blk_col, blk_col;
  int     n_col, n_ctl, ctl_pt;
  float   rr,gg,bb;
  int     gR[256], gG[256], gB[256];


  // direction (index) of color control 
  // 0 : 0   (constant)
  // 1 : 255 (constant)
  // 2 : increase 0 to 255 
  // 3 : decrease 255 to 0

// Rainbow color
  static int ctl[7][3]={{3,0,1},
                        {0,2,1},
                        {0,1,3},
                        {2,1,0},
                        {1,3,0},
                        {1,2,2},
			{1,1,1}
  };


// Blue to red
//  static int ctl[4][3]={{0,0,2},{2,2,1},{1,3,3},{1,0,0}};

  n_col     = 90;

  n_ctl     = 7-1;   // Raibow color

//n_ctl     = 4-1;    // blue to red

  n_blk_col = n_col/n_ctl;

  for(ic=0;ic<n_col;ic++){
    gR[ic]=0;  gG[ic]=0;  gB[ic]=0;
  }

  for(ctl_pt=0;ctl_pt<=n_ctl;ctl_pt++){

    if(ctl[ctl_pt][0]==0 || ctl[ctl_pt][0]==2) rr=0,0;
    if(ctl[ctl_pt][0]==1 || ctl[ctl_pt][0]==3) rr=1,0;
    if(ctl[ctl_pt][1]==0 || ctl[ctl_pt][1]==2) gg=0,0;
    if(ctl[ctl_pt][1]==1 || ctl[ctl_pt][1]==3) gg=1,0;
    if(ctl[ctl_pt][2]==0 || ctl[ctl_pt][2]==2) bb=0,0;
    if(ctl[ctl_pt][2]==1 || ctl[ctl_pt][2]==3) bb=1,0;

    gR[ctl_pt*n_blk_col]=(SYSMAX*rr);
    gG[ctl_pt*n_blk_col]=(SYSMAX*gg);
    gB[ctl_pt*n_blk_col]=(SYSMAX*bb);

    for(blk_col=1;blk_col<=n_blk_col-1;blk_col++){

      if(ctl[ctl_pt][0]==1)rr=SYSMAX*1.0;
      if(ctl[ctl_pt][1]==1)gg=SYSMAX*1.0;
      if(ctl[ctl_pt][2]==1)bb=SYSMAX*1.0;
      if(ctl[ctl_pt][0]==2)rr=SYSMAX*     (float)blk_col/(float)n_blk_col;
      if(ctl[ctl_pt][1]==2)gg=SYSMAX*     (float)blk_col/(float)n_blk_col;
      if(ctl[ctl_pt][2]==2)bb=SYSMAX*     (float)blk_col/(float)n_blk_col;
      if(ctl[ctl_pt][0]==3)rr=SYSMAX*(1.0-(float)blk_col/(float)n_blk_col);
      if(ctl[ctl_pt][1]==3)gg=SYSMAX*(1.0-(float)blk_col/(float)n_blk_col);
      if(ctl[ctl_pt][2]==3)bb=SYSMAX*(1.0-(float)blk_col/(float)n_blk_col);

      gR[ctl_pt*n_blk_col+blk_col]=rr;
      gG[ctl_pt*n_blk_col+blk_col]=gg;
      gB[ctl_pt*n_blk_col+blk_col]=bb;
    }
  }

  for(ic=0;ic<n_col;ic++){
    printf("%3d %3d %3d\n",gR[ic],gG[ic],gB[ic]);
  }

}
      
