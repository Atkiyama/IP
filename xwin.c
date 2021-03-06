#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define SIZE_WIN_X     960  // Window size         : Width
#define SIZE_WIN_Y     700  // Window size         : Height

#define MAX_PIXEL      512  // Max image size      : Width  ( < 512 )
#define MAX_LINE       512  // Max image size      : Height ( < 512 )

#define POS_IMG_X       30  // Image position      : X  (Upper-left)
#define POS_IMG_Y      140  // Image position      : Y  (Upper-left)

#define POS_CBAR_X     570  // Color bar position  : X  (Upper-left)
#define POS_CBAR_Y     200  // Color bar position  : Y  (Upper-left)
#define SIZE_CBAR_X     30  // Color bar size      : Width 
#define SIZE_CBAR_Y    360  // Color bar size      : Height

#define POS_HST_X      660  // Hist. Disp. Position : X  (Upper-left)
#define POS_HST_Y      380  // Hist. Disp. Position : Y  (Upper-left)
#define SIZE_HST_X     256  // Histgram image size  : Width
#define SIZE_HST_Y     256  // Histgram image size  ; Height

#define NUM_CLM_MENU    10  // Number of menu (Column)
#define NUM_ROW_MENU     2  // Number of menu (Row)

#define NCOLOR          90  // Number of color  (do not change !!)

#define MSG    "18-0107  Shuhei Akiyama"  // My name and student number 



// 文字列格納変数  メニュー表示内容
//                    01          02        03           04  ...   (max 10)
char *chr_menu[] = {" Japan  ","P.Color1","P.Color3 ","Img Disp","Add Noise ", "Gradient","mk_contour_ov_img","        ","        ","          ",
                    " Osaka  ","P.Color2","  Mono   ","Ave Filter","Med Filter", "Laplacian","unsharp_masking4","         ","        ","          "};
//                    11          12        13           14  ...   (max 20)




int   menu_xpos_start = 10;  // start position of menu  : X
int   menu_ypos_start = 10;  // start position of menu  : Y

int   menu_width  = 90;      // menu size : width
int   menu_height = 30;      // menu size : height

/****** for X WINDOW ********/ 
Display *d;   // ディスプレイ変数     : (X-Win)
Window   w;   // ウィンドウ変数       : (X-Win)
XEvent   e;   // イベント情報格納変数 : (X-Win)
Visual   *vis;
Colormap cm;

/****** for Image ******/
GC gc;        // Graphic Context
char *gimg;   // data for Imagemap
XImage *ximg; // Imagemap
int depth;

float gR[NCOLOR],gG[NCOLOR],gB[NCOLOR]; // used for color pallette
int   byte,bitmap_pad;


void dsp_window ();
void dsp_frame  (Display *d,Window w);
void dsp_image  (int img_org[MAX_LINE][MAX_PIXEL],int fmin,int fmax,int num_line,int num_pixel, int rgbset);
void dsp_composite_image(unsigned char band_image[4][MAX_LINE][MAX_PIXEL], int num_line, int num_pixel);
void dsp_hist   (int histgram[SIZE_HST_Y][SIZE_HST_X]);
void get_histval(int *fmin,int *fmax);
int  get_menu_information();

void set_color_pal(int rgbset);
unsigned long return_pix_from_RGB(float R,float G, float B);
void get_byte(int depth,int *byte, int *bitmap_pad);


void dsp_window()
{
     // ｘサーバと接続する(ディスプレイ情報の取得)
     d = XOpenDisplay(NULL);

     // ウィンドウを作成(親window変数:RootWindow(d,0),位置:(0,0),サイズ: 960x700
     w = XCreateSimpleWindow(d,RootWindow(d,0),0,0,
             SIZE_WIN_X ,SIZE_WIN_Y ,2,1,WhitePixel(d,0));

     // ウィンドウのマップ(貼付け)
     XMapWindow(d,w);
 
     vis=XDefaultVisual(d,0);  // ビジュアルクラスの取得

     depth=DefaultDepth(d,0);  // ディスプレイの表示ビット数を調べる

     if(depth < 16){
       printf("This program needs 16 or 24 bit True color mode\n\n");
       exit(1);
     }

     // イベントの設定
     //     複数のイベントマスクを指定: ウィンドウ切替え(ExposureMask), 
     //                                 ボタン押した時(ButtonPressMask) にイベント取得 
     XSelectInput(d,w,ExposureMask | ButtonPressMask);  
}


// Draw menu boxes

void dsp_frame(Display *d,Window w)
{
      int row,clm;
      int x_l_corner,x_r_corner, y_l_corner, y_r_corner;

      int menu_no;

      gc = XCreateGC(d,w,0,0);              // 新しいGCを作成する
      XSetForeground(d,gc,BlackPixel(d,0)); // ボックス,文字の色の設定(黒)
      
      // メニュー用白枠,メニュー内文字表示処理
      for (row=0; row<NUM_ROW_MENU; row++) {
	for (clm=0; clm<NUM_CLM_MENU; clm++){

	  x_l_corner = menu_xpos_start + clm*(menu_width + 5) ; // 左上 X 
	  y_l_corner = menu_ypos_start + row*(menu_height+10) ; // 左上 Y

          // 左上基準位置より，指定サイズ（幅，高さ）で四角を描画
	  XDrawRectangle(d,w,gc, x_l_corner, y_l_corner, menu_width,menu_height);

          // 表示文字列の配列要素を計算
	  menu_no = row*NUM_CLM_MENU + clm;
          // メニュー文字列表示
	  XDrawString(d,w,gc, x_l_corner+20, y_l_corner+20, chr_menu[menu_no], strlen(chr_menu[menu_no]) ); 
	}
      }

      // 名前，学籍番号表示（左下）
      XDrawString (d,w,gc, 10, SIZE_WIN_Y-10, MSG, strlen(MSG)); 
}



void dsp_image(int img_org[MAX_LINE][MAX_PIXEL],int fmin,int fmax,int num_line, int num_pixel, int rgbset)
{
       unsigned long ul_pix;  /* pixel value */
       int line, pixel, value, col_idx, ftmp,byte,bitmap_pad;
       char str[100];
       unsigned int image_width, image_height;
 

       printf("  In function dsp image\n");
       //       printf("     line=%d, pixel=%d\n", num_line, num_pixel);
       gc = XCreateGC(d,w,0,0);   /* 新しいGCを作成する */

       XSetForeground(d,gc,BlackPixel(d,0)); 

       set_color_pal(rgbset);
       get_byte(depth, &byte, &bitmap_pad);

       gimg=(unsigned char*)malloc(num_line*num_pixel*byte); // 指定バイト分の確保

       vis=XDefaultVisual(d,0); // ビジュアルクラスの取得


       image_width  = (unsigned int) num_pixel;  // 画像サイズ(width )
       image_height = (unsigned int) num_line;   // 画像サイズ(height)

       // 画像表示位置をあらかじめ消去
       XClearArea    (d,w,    POS_IMG_X, POS_IMG_Y, MAX_PIXEL+2, MAX_LINE+2,0);

       // 画像枠の表示
       XDrawRectangle(d,w,gc, POS_IMG_X, POS_IMG_Y, image_width+1, image_height+1);


       //
       // ---------------------- 画像表示部 --------------------------       
       //
       // XImage構造体の作成
       ximg = XCreateImage(d,vis, depth, ZPixmap,0,gimg, image_width, image_height, bitmap_pad,0);
    	  
       // 色割り当て処理
       for (line=0; line<num_line; line++){
	 for (pixel=0; pixel<num_pixel; pixel++){

	   if (img_org[line][pixel]>NCOLOR) img_org[line][pixel] = NCOLOR-1;
	   if (img_org[line][pixel]<     0) img_org[line][pixel] = 0;

	   // 各画素値
	   value = img_org[line][pixel];

	   // 各画素値を色パレットより24bit 形式でR,G,B値を指定
	   ul_pix = return_pix_from_RGB( gR[value], gG[value], gB[value] );	
	   
	   // ピクセル値の設定
	   XPutPixel(ximg,pixel,line, ul_pix); 
	 }
       }

       //  画像の描画
       XPutImage(d,w,gc,ximg, 0,0, POS_IMG_X+1, POS_IMG_Y+1, image_width, image_height);
   

       //
       // ---------------------- カラースケール表示部 --------------------------       
       //

       //  カラー(モノクロ)スケール用枠の表示
       XDrawRectangle(d,w,gc, POS_CBAR_X-1, POS_CBAR_Y-1,  SIZE_CBAR_X+1, SIZE_CBAR_Y+1 );

       // XImage構造体の作成
       ximg=XCreateImage(d,vis,depth,ZPixmap,0,gimg, SIZE_CBAR_X,SIZE_CBAR_Y,bitmap_pad,0);
       
       // カラースケール画像作成
       line=0;
       for(line=0; line<SIZE_CBAR_Y; line++){
	 value=line/4;
	 for(pixel=0; pixel<SIZE_CBAR_X; pixel++){
	   ul_pix = return_pix_from_RGB(gR[value], gG[value], gB[value]);	
	   XPutPixel(ximg,pixel,line, ul_pix); 
	 }
       } 

       // カラースケール画像描画      
       XPutImage(d,w,gc,ximg,0,0,POS_CBAR_X, POS_CBAR_Y, SIZE_CBAR_X,SIZE_CBAR_Y);

       // カラースケール用数値表示部クリア
       XClearArea  (d,w,    POS_CBAR_X+SIZE_CBAR_X+2, POS_CBAR_Y-15,40,SIZE_CBAR_Y+20,0); 

       // カラースケール用数値作成
       for(col_idx=0; col_idx<=NCOLOR; col_idx=col_idx+10 ){
         ftmp = (fmax-fmin)*col_idx/NCOLOR + fmin;
	 sprintf(str,"%3d",ftmp);
	 XDrawString(d,w,gc,POS_CBAR_X+45, POS_CBAR_Y+5+col_idx*SIZE_CBAR_Y/NCOLOR,str,strlen(str)); 
       } 

       XClearArea  (d,w,    600, SIZE_WIN_Y-30,60*6+20,60,0);
       XDrawString (d,w,gc, 600, SIZE_WIN_Y-10, "Press right mouse button on histgram to enter minmum value", 
		   strlen("Press right mouse button on histgram to enter minmum value"));
}



void dsp_composite_image(unsigned char band_image[4][MAX_LINE][MAX_PIXEL], int num_line, int num_pixel)
{
     unsigned long ul_pix;  /* pixel value */
     int line, pixel, value, col_idx, ftmp,byte,bitmap_pad;
     unsigned int image_height, image_width;
     char str[100];

     FILE *data[4];
     int ch;
     float r, g, b;

     image_height = (unsigned int) num_line;
     image_width  = (unsigned int) num_pixel;

 
     gc = XCreateGC(d,w,0,0);   /* 新しいGCを作成する */

     XSetForeground(d,gc,BlackPixel(d,0)); 
       
     get_byte(depth, &byte, &bitmap_pad);

     gimg=(unsigned char*)malloc(num_line*num_pixel*byte); /* 指定バイト分の確保 */

     vis=XDefaultVisual(d,0); /* ビジュアルクラスの取得 */

     XClearArea  (d,w,      POS_IMG_X, POS_IMG_Y, MAX_PIXEL+2, MAX_LINE+2,0);

     /*  (A) XImage構造体の作成 */

     ximg = XCreateImage(d,vis, depth, ZPixmap,0,gimg, num_pixel, num_line , bitmap_pad,0);
    	  
       /* 色割り当て処理 */
       for (line=0; line<num_line; line++){
	 for (pixel=0; pixel<num_pixel; pixel++){

	   r = ((float)band_image[2][line][pixel]-128.)*1. /(255.- 128.);
	   g = ((float)band_image[1][line][pixel]-128.)*1. /(255.- 128.);
	   b = ((float)band_image[0][line][pixel]-128.)*1. /(255.- 128.);

	   ul_pix = return_pix_from_RGB( r, g, b );
	   
	   /* (B)  ピクセル値の設定 */
	   XPutPixel(ximg,pixel,line, ul_pix); 
	 }
       }

       /*  (C) 画像の描画 */

       XPutImage(d,w,gc,ximg, 0,0, POS_IMG_X+1, POS_IMG_Y+1, num_pixel, num_line );
 

}


void dsp_hist(int histgram[SIZE_HST_X][SIZE_HST_Y])
{
      int pixel, line, k;
      int icol,byte,bitmap_pad;
  
      gc = XCreateGC(d,w,0,0);   /* 新しいGCを作成する */

      XSetForeground(d,gc,BlackPixel(d,0)); 


      get_byte(depth, &byte, &bitmap_pad);

      gimg=(unsigned char*)malloc(SIZE_HST_Y*SIZE_HST_X*byte); /* 指定バイト分の確保 */

      vis=XDefaultVisual(d,0); /* ビジュアルクラスの取得 */

      /*  ヒストグラム用白枠表示処理  */
      XDrawRectangle(d,w,gc, POS_HST_X-1,POS_HST_Y-1, SIZE_HST_X+1,SIZE_HST_Y+1 );

      ximg=XCreateImage(d,vis,depth,ZPixmap,0,gimg,SIZE_HST_X,SIZE_HST_Y,bitmap_pad,0);
  
      for (line=0; line<SIZE_HST_Y; line++){
	for (pixel=0; pixel<SIZE_HST_X; pixel++){
	  if (histgram[line][pixel]==   10) XPutPixel(ximg,pixel,line, BlackPixel(d,0)); 
	  if (histgram[line][pixel]==10000) XPutPixel(ximg,pixel,line, WhitePixel(d,0)); 
	}
      }
  
      XPutImage(d,w,gc,ximg,0,0,POS_HST_X,POS_HST_Y, SIZE_HST_X, SIZE_HST_Y );

      XClearArea(d,w,    POS_HST_X-  1,POS_HST_Y+SIZE_HST_Y, SIZE_HST_X+30,20, 0);

      XDrawString(d,w,gc,POS_HST_X-  2,POS_HST_Y+SIZE_HST_Y+13,  "0",strlen(  "0"));
      XDrawString(d,w,gc,POS_HST_X+250,POS_HST_Y+SIZE_HST_Y+13,"255",strlen("255"));
  
}





int get_menu_information(int *fmin, int *fmax)
{
     int menu_no; /* メニュー選択結果格納変数 */ 
     int row, clm; /* メニュー番号用変数 */ 
     int x_l_corner,x_r_corner, y_l_corner, y_r_corner;

     menu_no = 100; /* メニュー番号の初期値 */

     /* イベントの取得 */
     XNextEvent(d,&e); 
     dsp_frame (d, w);

     /* イベントのタイプがマウスクリック */
     if (e.type == ButtonPress){
       if (e.xbutton.button == 1){ /* マウス左クリックの時 = ボタンが１番目 */

	 for (row=0; row<NUM_ROW_MENU ; row++) { /*  メニューの数 */ 
	   for (clm=0; clm<NUM_CLM_MENU ; clm++){

	     x_l_corner = menu_xpos_start + clm*(menu_width + 5) ;
	     y_l_corner = menu_ypos_start + row*(menu_height+10) ;

	     x_r_corner = x_l_corner + menu_width;
	     y_r_corner = y_l_corner + menu_height;

	     if (x_l_corner <= e.xbutton.x && e.xbutton.x <= x_r_corner ){
	       if (y_l_corner <= e.xbutton.y && e.xbutton.y <= y_r_corner ){
		 menu_no = row*NUM_CLM_MENU+clm + 1; 
	       }
	     }

	   }
	 }

       }
       
       if(e.xbutton.button == 3){ /* マウス右クリックの時 = ボタンが３番目 */

	 /***********************************************/
	 /*  ヒストグラムから最小値と最大値をとる       */
	 /**********************************************/

	  if (POS_HST_X <= e.xbutton.x && e.xbutton.x <= (POS_HST_X + SIZE_HST_X) &&
	      POS_HST_Y <= e.xbutton.y && e.xbutton.y <= (POS_HST_Y + SIZE_HST_Y) ) {

	    *fmin = e.xbutton.x - POS_HST_X;
	    if (*fmin < 0) *fmin = 0;

	    XSetLineAttributes(d, gc, 3, LineSolid, CapButt, JoinMiter);
	    XDrawLine(d,w,gc,e.xbutton.x,POS_HST_Y+255,e.xbutton.x,POS_HST_Y+255+10);
	
	    XClearArea  (d,w,    600, SIZE_WIN_Y-30,60*6+20,60,0);
	    XDrawString (d,w,gc, 800, SIZE_WIN_Y-10, "Enter maximum value", strlen("Enter maximum value")); 
	  }  

	  XNextEvent(d,&e);
	  if (POS_HST_X <= e.xbutton.x && e.xbutton.x <= (POS_HST_X + SIZE_HST_X) &&
	      POS_HST_Y <= e.xbutton.y && e.xbutton.y <= (POS_HST_Y + SIZE_HST_Y) ) {

	    *fmax = e.xbutton.x - POS_HST_X;
	    if (*fmin > 255) *fmax = 255;

	    XSetLineAttributes(d, gc, 3, LineSolid, CapButt, JoinMiter);
	    XDrawLine(d,w,gc,e.xbutton.x,POS_HST_Y+255,e.xbutton.x,POS_HST_Y+255+10);

	  }  
	  XClearArea  (d,w,    800, SIZE_WIN_Y-30,150,60,0);

	  printf("    min=%d max=%d\n",*fmin,*fmax);
	  //printf("x=%d y=%d\n",e.xbutton.x,e.xbutton.y);	
       }
     } else if(e.type == Expose ){
       printf("EXPOSE\n");
     }
     return menu_no;
}




void set_color_pal(rgbset)
{
  /* set color pallette : gR(red),gG(green),gB(blue)  */
  /* NCOLOR : number of color pallette                */
  /* rgbset=0;  : カラースケール    */
  /* rgbset=1;  : 白黒濃淡スケール  */

     int i,j,k;
     int R[NCOLOR],G[NCOLOR],B[NCOLOR];
     FILE *color_tbl;
     char fname[4][200]={"color.txt", "color2.txt","color3.txt", "mono.txt"};/* color palette file */

     color_tbl=fopen(fname[rgbset],"r"); /* color palette file open */
     if (color_tbl == NULL) {
       printf ("    check color table file\n");
       exit(-1);
     }
     for(j=0; j<NCOLOR; j++){
       fscanf(color_tbl,"%d %d %d",&R[j],&G[j],&B[j]);
     }
     fclose(color_tbl);

     for(j=0; j<NCOLOR; j++){
       gR[j]=(float)R[j]/255;
       gG[j]=(float)G[j]/255;
       gB[j]=(float)B[j]/255;
       //            printf("%f %f %f\n", gR[j],gG[j],gB[j]);
     }
}

unsigned long return_pix_from_RGB(float R,float G, float B)
{
  /* input : R,G,B (0-1:float)                   */
  /* output : pixel value for True Color         */

     int max_color;

     max_color = (2 << (depth/3-1));
     // printf("depth=%d color=%d\n",depth,max_color);

     R=R*(max_color-1); 
     G=G*(max_color-1); 
     B=B*(max_color-1); /* to 0-255 (24bit), 0-63 (16bit)*/
     // printf("%f %f %f\n", R,G,B);

     /* 色番号(ピクセル値)の計算 */
     if (depth == 16) 
       return (int)R*max_color*max_color*2           + (int)G*max_color*2 + (int)B;
     else if (depth == 24)
       return (int)R*max_color*max_color             + (int)G*max_color   + (int)B;
     else if (depth ==  8) 
       return (int)R*max_color*max_color*max_color*8 + (int)G*max_color  + (int)B;
}


void get_byte(int depth, int *byte, int *bitmap_pad)
{
       if (depth==8) {
	 *byte=1; *bitmap_pad=8;
       }else if (depth == 16){
	 *byte=2; *bitmap_pad=16;
       }else if (depth == 24){
	 *byte=4; *bitmap_pad=8;
       }

}
