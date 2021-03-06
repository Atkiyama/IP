/**********************************************************************

Compile : cc -O -o imgdisp sat_main.o xwin.o sub.o 
             -I/usr/X11R6/include  -L/usr/X11R6/lib -lX11 -lm 

          ( make )

run     :  ./imgdisp
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define    MAX_PIXEL      512  // 最大画像サイズ:(Width)
#define    MAX_LINE       512  // 最大画像サイズ:(Height)

#define    SIZE_HST_X     256  // ヒストグラム サイズ:(Width)
#define    SIZE_HST_Y     256  // ヒストグラム サイズ:(Height)

/*** in xwin.c ***/
void dsp_image(int img_org[MAX_LINE][MAX_PIXEL],int fmin,int fmax, int num_line, int num_pixel, int rgbset);

//void dsp_composite_image(unsigned char band_image[4][MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
void dsp_window();
int  get_menu_information(int *fmin,int *fmax);

/*** in sub.c ***/
void read_NOAA_data   (unsigned char img_org      [MAX_LINE][MAX_PIXEL], int *num_line, int *num_pixel);
//void read_AVNIR_data  (unsigned char band_image[4][MAX_LINE][MAX_PIXEL], int *num_line, int *num_pixel);

void linear           (unsigned char img_org[MAX_LINE][MAX_PIXEL], int fmin, int fmax,int img_new[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
void mk_histgram      (unsigned char img_org[MAX_LINE][MAX_PIXEL], int histgram[SIZE_HST_Y][SIZE_HST_X], int num_line, int num_pixel);
void add_noise        (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
void averaged_filter  (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
void median_filter    (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
void edge_sobel       (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
void edge_laplacian   (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
void unsharp_masking4  (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
//void mk_contour       (unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
void mk_contour_ov_img(unsigned char img_org[MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);



int main(void)
{
    unsigned char   img_org      [MAX_LINE][MAX_PIXEL]; // NOAA データ用配列
    int             img_new      [MAX_LINE][MAX_PIXEL]; // NOAA 表示用データ配列

    unsigned char   band_image[4][MAX_LINE][MAX_PIXEL]; // AVNIR 多波長画像用配列

    int             num_line, num_pixel;                // 画像サイズ用

    int             rgbset,menu_no;                     // カラーバー, メニュー選択結果格納変数
    int             histgram[SIZE_HST_Y][SIZE_HST_X];   // ヒストグラム画像表示用
    int             fmin,fmax;                          // 表示時の最小，最大値


    fmin=0; fmax=255; // 表示画像の最小値，最大値

    rgbset=0;         // カラー設定(0-2: 擬似カラー，3: グレースケール
    
    read_NOAA_data(img_org,&num_line,&num_pixel);  // NOAA ファイル関数
    
    printf("line = %d, pixel = %d\n", num_line,num_pixel);
    
    dsp_window();    // Window 作成など
    
    while(1){        // イベント取得を何度も実施
      
      // menu の位置から処理番号を取得
      menu_no = get_menu_information(&fmin, &fmax);

      printf("menu =%d\n", menu_no);	

      /*********************************/
      /* 処理内容呼出し用 if文追加！！ */
      /*********************************/

      if(menu_no== 1) {         // Menu 1: 日本画像読込
	read_NOAA_data(img_org,&num_line,&num_pixel);  // NOAA 画像データ
	fmin=0; fmax=255;
      }

      if(menu_no== 2) rgbset=0;  // 擬似カラー1
      if(menu_no==12) rgbset=1;  // 擬似カラー2
      if(menu_no== 3) rgbset=2;  // 擬似カラー3
      if(menu_no==13) rgbset=3;  // 白黒濃淡
      
      if(menu_no==4){            // Menu  4  画像表示 
		linear      (img_org,fmin,fmax,img_new ,num_line,num_pixel); // 線形濃度変換
		dsp_image   (img_new,fmin,fmax,         num_line,num_pixel, rgbset);// 画像表示 
		mk_histgram (img_org,          histgram,num_line,num_pixel); // ヒストグラム計算
		dsp_hist    (histgram);                                      // ヒストグラム表示 
      }
      if(menu_no==5)  add_noise(img_org, num_line,num_pixel);
      if(menu_no==6)  edge_sobel(img_org, num_line,num_pixel);
      if(menu_no==7) mk_contour_ov_img (img_org, num_line, num_pixel);
      if(menu_no==14) averaged_filter(img_org, num_line, num_pixel);
      if(menu_no==15) median_filter(img_org, num_line,num_pixel);
      if(menu_no==16) edge_laplacian(img_org, num_line,num_pixel);
      if(menu_no==17) unsharp_masking4(img_org, num_line,num_pixel);
      
    }
    return 0;
}

