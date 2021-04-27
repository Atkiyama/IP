#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define    MAX_PIXEL      512  // 最大画像サイズ:(Width)
#define    MAX_LINE       512  // 最大画像サイズ:(Height)

#define    SIZE_HST_X     256  // ヒストグラム サイズ:(Width)
#define    SIZE_HST_Y     256  // ヒストグラム サイズ:(Height)



void read_NOAA_data(unsigned char img_org[MAX_LINE][MAX_PIXEL], int *num_line, int *num_pixel)
{
   // NOAA AVHRR 画像（日本付近）の読込み
   // 赤外画像

   FILE *data;
   int line;

    printf("  In function read NOAA data\n");

    data = fopen( "d850429.avhrr4" , "r" );
    if (data == NULL) {
        printf ("    no data in the directory \n");
        exit(-1);
    }

    // skip the 512 bytes header 
    fseek (data, 512L, SEEK_SET);

    *num_line = 480;
    *num_pixel= 512;

    for (line=0; line < *num_line; line++) {
       fread( img_org[line], *num_pixel, 1, data );
    }
    fclose(data);
}

void read_AVNIR_data(unsigned char band_image[4][MAX_LINE][MAX_PIXEL], int *num_line, int *num_pixel)
{
   // AVNIR 画像（大阪城近辺）の読込み
   // 可視波長 ４バンド

    FILE *data[4];
    int line, band;

    char fn[4][20]={"d970510b1.avnir","d970510b2.avnir","d970510b3.avnir","d970510b4.avnir"};

    printf("  In function read AVNIR data\n");

    *num_line = 500;
    *num_pixel= 500;

    for(band=0; band<4; band++) {
       printf("    Channel [%d] = %s\n", band+1, fn[ band ]);
       data[ band ] = fopen( fn[ band ] , "r" );
       if (data[ band ] == NULL) {
	 printf ("    no data in the directory \n");
	 exit(-1);
       }

       for (line=0; line< *num_line; line++) {
	 fread( band_image[ band ][ line ], *num_pixel, 1, data[ band ] );
       }
       fclose(data[ band ]);
    }

}

void linear(unsigned char img_org[MAX_LINE][MAX_PIXEL],int fmin, int fmax,int img_new[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel)
{
    // 線形濃度変換
    // a, b : 入力画像の最小値，最大値  
    // c, d : 変換画像の最小値，最大値（色割当ては０〜８９（９０色）なので，０，８９）

    int line,pixel;
    int new_val;
    int diff;
    diff=fmax-fmin;
    printf("  In function linear\n");


      for(line=0;line<num_line;line++){
	for(pixel=0;pixel<num_pixel;pixel++){
	  new_val=(img_org[line][pixel]-fmin)*89/diff;
	  if(new_val<0)new_val=0;
	  if(new_val>89)new_val=89;
	  img_new[line][pixel]=new_val;
	 
	}
      }


}

void mk_histgram(unsigned char img_org[MAX_LINE][MAX_PIXEL],int histgram[SIZE_HST_Y][SIZE_HST_X], int num_line, int num_pixel)
{

      int hist[256];
      int line, pixel;
      int value;
      int maxhist, normalized_hist;
      int i;

      printf("  In function make histgram\n");
      //      printf("  line = %d, pixel = %d\n", num_line, num_pixel);

  // 課題 配列 hist にヒストグラムを作成する．
  // ヒストグラムの分割数は256とし，（濃度値に対応）させる

      for(value=0; value<256; value++) hist[value]=0;

      for(line=0;line<num_line;line++){
	for(pixel=0;pixel<num_pixel;pixel++){
	  hist[img_org[line][pixel]]=hist[img_org[line][pixel]]+1;
	}
      }


     	for(value=0; value<256; value++){
         printf("%d\n",hist[value]);
	}

     // hist を用いて，グラフ化を実施

      maxhist = -9999;
      for (value=1; value<255; value++) {
	if (hist[value]>maxhist) maxhist=hist[value];
      }

      for (line=0; line<SIZE_HST_Y; line++){
	for (pixel=0; pixel<SIZE_HST_X; pixel++) histgram[line][pixel]=10000;  
      }

      printf("    max_count = %d\n",maxhist);

      if (maxhist >0) {
	for (value=0; value<256; value++){
 
	  normalized_hist = hist[value]*255 / maxhist;


	  if (normalized_hist > 255) normalized_hist=255;
	  if (normalized_hist <   0) normalized_hist=  0;

	  for (i=0; i<normalized_hist; i++){
	    if ((255-i)>=0 && (255-i)<255) histgram[255-i][value]=10; 

	  }

	}
      }
 
      for (i=0; i<256; i++){
       histgram[255][i]=10;
      }
      
}


//
// 疑似のイズ作成
//
void add_noise (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel)
{
  int   n,iseed,i,pixel,line;
  printf("  In function add noises \n");

  // make program here
  n=300;
  iseed = 1;
  srand(iseed);
  for(i=1;i<=n;i++){
    pixel = (rand()/(float)RAND_MAX)*(num_pixel-1);
    line  = (rand()/(float)RAND_MAX)*(num_line-1);
    img_org[line][pixel] =0;
  }

}




void averaged_filter(unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel)
{
  int   line, pixel, iy, ix;
  int   img_tmp[MAX_LINE][MAX_PIXEL], sum;

  printf("  In function averaged_filter\n");

  // Step 1
  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++) 
      img_tmp[ line ][ pixel ] = img_org[ line ][ pixel ] ;

  // Step 2
  for (line=+1 ; line < num_line-1 ; line++) {
    for (pixel=+1 ; pixel < num_pixel-1 ; pixel++) {
      sum = 0;
      for (iy=-1; iy<=+1; iy++)
	for (ix=-1; ix<=+1; ix++)
	  sum = sum + img_org[ line+iy ][ pixel + ix] ;
      img_tmp[ line ][ pixel ] = sum / 9;
    }
  }

  // Step 3
  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++) 
      img_org[ line ][ pixel ] = img_tmp[ line ][ pixel ] ;


}



void median_filter(unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel)
{
  int  line, pixel, iy, ix;
  int  window [9], tmp, i, j,k,sum;
  int  img_tmp[MAX_LINE][MAX_PIXEL];


 printf("  In function median_filter\n");


   // Step 1
  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++) 
      img_tmp[ line ][ pixel ] = img_org[ line ][ pixel ] ;

  // Step 2
  for (line=+1 ; line < num_line-1 ; line++) {
    for (pixel=+1 ; pixel < num_pixel-1 ; pixel++) {
      sum = 0;
      for (iy=-1; iy<=+1; iy++){
	for (ix=-1; ix<=+1; ix++){
	  window[sum] =img_org[ line+iy ][ pixel + ix] ;
	  sum++;
	}
      }
      //バブルソート
      for(j=0;j<9;j++){
	for(i=0;i<9-j;i++){
	  if(window[i]>window[i+1]){
	    k=window[i];
	    window[i]=window[i+1];
	    window[i+1]=k;
	  }
	}
      }
      //printf("%d,%d,%d,%d,%d,%d,%d,%d,%d\n",window[0],window[1],window[3],window[4],window[5],window[6],window[7],window[8],window[9]);
      img_tmp[line][pixel]=window[4];
      
    }
  }
 


  // Step 3
  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++) 
      img_org[ line ][ pixel ] = img_tmp[ line ][ pixel ] ;
 
     
 
  // make program here
}
void edge_laplacian (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel)
{
  int   line, pixel, iy, ix;
  int   img_tmp [MAX_LINE][MAX_PIXEL], img_tmp2[MAX_LINE][MAX_PIXEL];
  int   img_tmp3[MAX_LINE][MAX_PIXEL], img_tmp4[MAX_LINE][MAX_PIXEL];
  int   sum_l, sum_sx, sum_sy, i, edge;
  int   ope_sx[] = {-1, 0, 1,
                   -2, 0, 2, 
                   -1, 0, 1};

  int   ope_sy[] = {-1,-2,-1,
                    0, 0, 0, 
                    1, 2, 1};

  int   ope_l[] = { 0, 1, 0,
                    1,-4, 1, 
                    0, 1, 0};
  printf("  In function laplacian\n");

  // Step 1
  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++)  {
      img_tmp [ line ][ pixel ]=0; img_tmp2[ line ][ pixel ]=0; 
      img_tmp3[ line ][ pixel ]=0; img_tmp4[ line ][ pixel ]=0;
    }

  // Step 2 
  for (line=+1 ; line < num_line-1 ; line++) {
    for (pixel=+1 ; pixel < num_pixel-1 ; pixel++) {
      i=0; sum_l=0;
      for (iy=-1; iy<=+1; iy++) {
	for (ix=-1; ix<=+1; ix++) {
	  sum_l = sum_l + img_org[ line+iy ][ pixel + ix] * ope_l[i]; // laplacian
	  i++;
	}
      }
      img_tmp[ line ][ pixel ] = sum_l;
    }
  }
  // Step 3 
  for (line=+1 ; line < num_line-1 ; line++) {
    for (pixel=+1 ; pixel < num_pixel-1 ; pixel++) {
      
      if(img_tmp[line][pixel] == 0){
	//画素値が0の場合
	for (iy=-1; iy<=+1; iy++) {
	  for (ix=-1; ix<=+1; ix++) {
	    if(img_tmp[line+ix][pixel+iy] != 0 ){
	      img_tmp2[line][pixel] =1;
	    }
	  }
	}
 
      }else if(img_tmp[line][pixel] >0){
	//画素値が正の数の時の処理
	for (iy=-1; iy<=+1; iy++) {
	  for (ix=-1; ix<=+1; ix++) {
	    if(img_tmp[line+ix][pixel+iy] < 0 ){
	      img_tmp2[line][pixel] =1;
	    }   
	  }
	}
 
      }else if(img_tmp[line+ix][pixel + iy]<0){
	//画素値が負の数の時の処理
	 for (iy=-1; iy<=+1; iy++) {
	  for (ix=-1; ix<=+1; ix++) {
	    if(img_tmp[line+ix][pixel+iy] > 0 ){
	      img_tmp2[line][pixel] =1;
	    }   
	  }
	}
      }else
	img_tmp2[line][pixel]=0;

    }
  }


  // Step 4
  for (line=+1 ; line < num_line-1 ; line++) {
    for (pixel=+1 ; pixel < num_pixel-1 ; pixel++) {
      i=0; sum_sx=0; sum_sy=0;
      for (iy=-1; iy<=+1; iy++) {
	for (ix=-1; ix<=+1; ix++) {
	  sum_sx = sum_sx + img_org[ line+iy ][ pixel + ix] * ope_sx[i]; // x direction
	  sum_sy = sum_sy + img_org[ line+iy ][ pixel + ix] * ope_sy[i]; // y direction
	  i++;
	}
      }
      edge  = sqrt( sum_sx*sum_sx + sum_sy*sum_sy );
      if (edge > 255) edge = 255;
      if (edge <   0) edge = 0;
      img_tmp3[ line ][ pixel ] = edge;
    }
  }

  // Step 5
  for (line=0; line < num_line; line++) {
    for (pixel=0; pixel < num_pixel; pixel++) {
      if ( img_tmp2[line][pixel] > 0 && img_tmp3[line][pixel] >= 40) 
	img_tmp4 [ line ][ pixel ] = 255;
      else 
	img_tmp4 [ line ][ pixel ] = 0;
    }
  }

  // Step 6 
  for (line=0; line < num_line; line++)
    for (pixel=0; pixel < num_pixel; pixel++)
      img_org[line][pixel] = img_tmp4[line][pixel];
}



void edge_sobel (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel)
{

  int   line, pixel, iy, ix;
  int   img_tmp[MAX_LINE][MAX_PIXEL];
  int   sum_x, sum_y, i, edge;
  int   ope_x[] = {-1, 0, 1, 
                   -2, 0, 2, 
                   -1, 0, 1};
  int   ope_y[] = {-1,-2,-1,
		    0, 0, 0,
		    1, 2, 1};

  printf("  In function edge detection\n");

  // Step 1
  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++) 
      img_tmp[ line ][ pixel ] = 0;

  // Step 2
  for (line=+1 ; line < num_line-1 ; line++) {
    for (pixel=+1 ; pixel < num_pixel-1 ; pixel++) {
      i=0; sum_x=0; sum_y = 0;
      for (iy=-1; iy<=+1; iy++) {
	for (ix=-1; ix<=+1; ix++) {
	  sum_x = sum_x + img_org[ line+iy ][ pixel + ix] * ope_x[i]; // ｘ方向のエッジのみ計算
	  sum_y = sum_y + img_org[ line+iy ][ pixel + ix] * ope_y[i]; // y方向のエッジのみ計算
	  i++;
	}
      }
      edge  = sqrt( sum_x*sum_x + sum_y*sum_y );
      if (edge > 255) edge = 255;
      if (edge <   0) edge = 0;
      img_tmp[ line ][ pixel ] = edge;
    }
  }

  // Step 3
  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++) 
      img_org[ line ][ pixel ] = img_tmp[ line ][ pixel ] ;

}


void  mk_contour_ov_img(unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel)
{
  int   line, pixel, Level_Slice;
  int   img_tmp [MAX_LINE][MAX_PIXEL], img_tmp2[MAX_LINE][MAX_PIXEL];
  int   img_LS  [MAX_LINE][MAX_PIXEL];
  int   sum_l;
  int   ope_l[] = { 0, 1, 0,
                    1,-4, 1, 
                    0, 1, 0};

  printf("  In function mk contour over image\n");

  // Step 1
  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++)  {
      img_tmp [ line ][ pixel ]=0; img_tmp2[ line ][ pixel ]=0; 
    }

  // Step 2
  for (Level_Slice=16; Level_Slice<=240; Level_Slice += 16) {
    // ------------------
    // 2a Level Slice image 
    for (line=0 ; line < num_line  ; line++) {
      for (pixel=0 ; pixel < num_pixel ; pixel++) {
  	if (img_org[line][pixel] > Level_Slice) 
	  img_LS [ line ][ pixel ] = 1;
	else 
	  img_LS [ line ][ pixel ] = 0;
      }
    }
    // ------------------
    //  2b Laplacian 4
    for (line=+1 ; line < num_line-1 ; line++) {
      for (pixel=+1 ; pixel < num_pixel-1 ; pixel++) {
	sum_l =// img_LS[ line-1 ][ pixel -1]   * ope_l[0]
                 +img_LS[ line-1 ][ pixel   ] //* ope_l[1]
   	       //+img_LS[ line-1 ][ pixel +1]   * ope_l[2]
                 +img_LS[ line   ][ pixel -1] //* ope_l[3]
	         +img_LS[ line   ][ pixel   ]   * ope_l[4]
                 +img_LS[ line   ][ pixel +1] //* ope_l[5]
               //+img_LS[ line+1 ][ pixel -1]   * ope_l[6]
         	 +img_LS[ line+1 ][ pixel   ] //* ope_l[7]
  	       //+img_LS[ line+1 ][ pixel +1]   * ope_l[8]
                  ;
	img_tmp [ line ][ pixel ] = sum_l;
      }
    }

    // ------------------
    // 2c draw contours
    for (line=+1 ; line < num_line-1 ; line++) 
      for (pixel=+1 ; pixel < num_pixel-1 ; pixel++) 
	if ( img_tmp [ line ] [pixel ]  > 0 ) img_tmp2 [ line ][ pixel ] = 255;

  } // end of Level Slice

  //---------------------------------------
  //  ここで原画像との重ねあわせが必要
  //  以下のプログラムは当値線のみを表示している

  //  具体的には、img_tmp2 の全画素に対し，値を調べ，
  //  １以上であれば，img_org を 0 （黒色）とする
  //---------------------------------------

  for (line=0 ; line < num_line  ; line++) {
    for (pixel=0 ; pixel < num_pixel ; pixel++) {
      if(img_tmp2[line][pixel]>=1)
	img_org [ line ] [ pixel ] =0;
    }
  }

}
void unsharp_masking4 (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel)
{
  int   line, pixel, iy, ix;
  int   img_tmp[MAX_LINE][MAX_PIXEL];
  int   sum, i;
  int   ope_l[] = { 0,-1, 0,
                   -1, 5,-1, 
                    0,-1, 0};

  printf("  In function unsharpmasking 4 \n");

  // Step 1
  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++)
      img_tmp [ line ][ pixel ]=0;

  // Step 2 
  for (line=+1 ; line < num_line-1 ; line++) {
    for (pixel=+1 ; pixel < num_pixel-1 ; pixel++) {
      i=0; sum=0;
      for (iy=-1; iy<=+1; iy++) {
	for (ix=-1; ix<=+1; ix++) {
	  sum = sum + img_org[ line+iy ][ pixel + ix] * ope_l[i]; // USM 4
	  i++;
	}
      }
      img_tmp[ line ][ pixel ] = sum;
    }
  }

  for (line=0; line < num_line; line++) 
    for (pixel=0; pixel < num_pixel; pixel++) {
      if ( img_tmp [ line ][ pixel ] > 255 ) img_tmp [ line ][ pixel ] = 255;
      if ( img_tmp [ line ][ pixel ] <   0 ) img_tmp [ line ][ pixel ] =   0;
      img_org[line][pixel] = img_tmp[line][pixel];
    }

}
