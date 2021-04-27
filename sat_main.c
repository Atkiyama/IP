/**********************************************************************

Compile : cc -O -o imgdisp sat_main.o xwin.o sub.o 
             -I/usr/X11R6/include  -L/usr/X11R6/lib -lX11 -lm 

          ( make )

run     :  ./imgdisp
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define    MAX_PIXEL      512  // �ő�摜�T�C�Y:(Width)
#define    MAX_LINE       512  // �ő�摜�T�C�Y:(Height)

#define    SIZE_HST_X     256  // �q�X�g�O���� �T�C�Y:(Width)
#define    SIZE_HST_Y     256  // �q�X�g�O���� �T�C�Y:(Height)

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
    unsigned char   img_org      [MAX_LINE][MAX_PIXEL]; // NOAA �f�[�^�p�z��
    int             img_new      [MAX_LINE][MAX_PIXEL]; // NOAA �\���p�f�[�^�z��

    unsigned char   band_image[4][MAX_LINE][MAX_PIXEL]; // AVNIR ���g���摜�p�z��

    int             num_line, num_pixel;                // �摜�T�C�Y�p

    int             rgbset,menu_no;                     // �J���[�o�[, ���j���[�I�����ʊi�[�ϐ�
    int             histgram[SIZE_HST_Y][SIZE_HST_X];   // �q�X�g�O�����摜�\���p
    int             fmin,fmax;                          // �\�����̍ŏ��C�ő�l


    fmin=0; fmax=255; // �\���摜�̍ŏ��l�C�ő�l

    rgbset=0;         // �J���[�ݒ�(0-2: �[���J���[�C3: �O���[�X�P�[��
    
    read_NOAA_data(img_org,&num_line,&num_pixel);  // NOAA �t�@�C���֐�
    
    printf("line = %d, pixel = %d\n", num_line,num_pixel);
    
    dsp_window();    // Window �쐬�Ȃ�
    
    while(1){        // �C�x���g�擾�����x�����{
      
      // menu �̈ʒu���珈���ԍ����擾
      menu_no = get_menu_information(&fmin, &fmax);

      printf("menu =%d\n", menu_no);	

      /*********************************/
      /* �������e�ďo���p if���ǉ��I�I */
      /*********************************/

      if(menu_no== 1) {         // Menu 1: ���{�摜�Ǎ�
	read_NOAA_data(img_org,&num_line,&num_pixel);  // NOAA �摜�f�[�^
	fmin=0; fmax=255;
      }

      if(menu_no== 2) rgbset=0;  // �[���J���[1
      if(menu_no==12) rgbset=1;  // �[���J���[2
      if(menu_no== 3) rgbset=2;  // �[���J���[3
      if(menu_no==13) rgbset=3;  // �����Z�W
      
      if(menu_no==4){            // Menu  4  �摜�\�� 
		linear      (img_org,fmin,fmax,img_new ,num_line,num_pixel); // ���`�Z�x�ϊ�
		dsp_image   (img_new,fmin,fmax,         num_line,num_pixel, rgbset);// �摜�\�� 
		mk_histgram (img_org,          histgram,num_line,num_pixel); // �q�X�g�O�����v�Z
		dsp_hist    (histgram);                                      // �q�X�g�O�����\�� 
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
