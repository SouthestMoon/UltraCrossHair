#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define _MATH_DEFINES_DEFINED
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "./stb_image_write.h"
#include "./stb_truetype.h"
#include "./log.h"


//子弹默认参数
#define TIME  0            
#define X     0
#define Y     0.045
#define VX    880
#define VY    0 
#define DRAG  0.0025
//程序默认参数
#define  TICK         0.00555555555555555555555555555556   // 帧间隔时间（秒）180FPS
#define  TOLERANCE    0.0001                              // 二分法求上抬角度时的精度
#define  START_FROM   175                                // 起始绘制距离，第一道标尺
#define  END_AT        800                                // 终止绘制距离，最后一道标尺
#define  GAP          25                                // 每个标尺之间的距离  
//游戏默认参数                    
#define SCREEN_HEIGHT  1440           // 屏幕高度,像素为单位
#define FOV            55            // 垂直视场角
#define ENLARGEMENT    8            // 镜子放大倍数
#define ZERO_IN_DISTANCE  75       //归零距离
#define ZERO_IN_EXISTS    true     //归零是否存在
//默认标尺颜色 黑色
#define DEAFALTCOLOR_R 0
#define DEAFALTCOLOR_G 0
#define DEAFALTCOLOR_B 0
#define DEAFALTCOLOR_A 255

//默认刻度宽高
#define DEFAULTSCALEHEIGHT_MEETS50  1
#define DEFAULTSCALELENGTH_MEETS50  8
#define DEFAULTSCALEHEIGHT_MEETS100  2
#define DEFAULTSCALELENGTH_MEETS100  13
#define DEFAULTSCALEHEIGHT_NORMAL  1
#define DEFAULTSCALELENGTH_NORMAL  4

//默认输出图片大小
#define IMGWIDTH  1080
#define IMGHEIGHT 1080

// 用户自定义参数
// 子弹参数
double mTime = 0;
double x = 0;
double y = 0.045;
double vx = 820;
double vy = 0;
double drag = 0.002;
// 程序参数
double tick = 0.00555555555555555555555555555556; // 帧间隔时间（秒）180FPS
double tolerance = 0.0001;                       // 二分法求上抬角度时的精度
short startFrom = 175;                          // 起始绘制距离，第一道标尺
short endAt = 800;                             // 终止绘制距离，最后一道标尺
short gap = 25;                               // 每个标尺之间的距离
#warning 标尺数不应多于255
// 游戏参数
unsigned short screenHeight = 1440;       // 屏幕高度,像素为单位
unsigned short fov = 55;                 // 垂直视场角
unsigned short zeroInDistance = 75;     // 归零距离
double enlargement = 10;               // 镜子放大倍数
bool zeroInExists = true;             // 归零是否存在
//用户自定义颜色
char customColor_r = 0;
char customColor_g = 0;
char customColor_b = 0;
char customColor_a = (char)255;

//自定义标尺大小
char customScaleHeight_meets50 = 1;
char customScaleLength_meets50 = 8;
char customScaleHeight_meets100 = 2;
char customScaleLength_meets100 = 13;
char customScaleHeight_normal = 1;
char customScaleLength_normal = 4;
#warning 长宽均有限制
// 自定义图片参数
short imgWidth = 1080;
short imgHeight = 1080;
#warning 不得超过1440*1440
typedef struct {
  double x;
  double y;
  double vx;
  double vy;
  double drag;
  double dropPixel;
} bulletState; // 弹道状态

//每个像素数据，方便理解
typedef struct {
   char R;
   char G;
   char B;
   char A;
} pixel;

typedef struct {
  unsigned short distance;
  unsigned short drop;
} scale;

// 写入日志文件
void logWrite(const char *message, const char logLevel);
//将二维像素数组转换为stb_image_write可以接受的一维数组
char *pixelsToArray(pixel **array2d);
//画线函数，参数为二维像素数组，起点坐标，终点坐标，颜色
bool stb_DrawLine(pixel **img, int x0, int y0, int x1, int y1, pixel color);
// 重置弹道
void reset(bulletState* bullet);
// 三个中最接近给定距离的弹道数据
double closestIn3(const bulletState* bullet,const unsigned short* targetX);
// 弹道状态更新函数
inline bulletState* flush(bulletState* bullet);


// 二分法求上抬角度,返回上抬的角度
double stimulation(bulletState* bullet) {
  double AngleMax = 30;
  double AngleMin = 0;
  double yAtZeroIn = 1;
  
    for (reset(bullet); fabs(yAtZeroIn) >= tolerance;reset(bullet)) {
      bullet->vy = -bullet->vx * sin((AngleMax+AngleMin)/2 / 180 * M_PI); // 弹道初速度
      bullet->vx = bullet->vx * cos((AngleMax+AngleMin)/2 / 180 * M_PI); // 弹道初速度
      bulletState temp[3]={0};
      flush(bullet);
      do {  //此处有陷入死循环的嫌疑，后期改造，速度够快时不会该帧子弹不会落在该区间
        temp[0] = temp[1];
        temp[1] = temp[2];
        temp[2] = *bullet;
        //printf("x1:%f, drop1:%f, x2:%f, drop2:%f\n",temp[1].x,temp[1].dropPixel,temp[2].x,temp[2].dropPixel);
        flush(bullet);

      }while (temp[1].dropPixel!=closestIn3(temp, &zeroInDistance)); 


      if(bullet->y<=0) AngleMax=(AngleMax+AngleMin)/2;
      else AngleMin=(AngleMax+AngleMin)/2;
      yAtZeroIn = bullet->y;
      //printf("yAtZeroIn: %f, AngleMax: %f, AngleMin: %f\n", yAtZeroIn, AngleMax, AngleMin);
    }
    logWrite("归零已校准", 'I');
  return  (AngleMax + AngleMin) / 2;
}





void outputImg(const scale * data) {

  char* img = nullptr;
  unsigned short scalesNumber=0;//当前绘制到第几根标尺
  //分配二位数组内存
  pixel **pixels = malloc(sizeof(pixel*) * imgWidth);
  for (short i = 0; i < imgWidth; i++) {
    pixels[i] = malloc(sizeof(pixel) * imgHeight);
    if(i==imgWidth-1)
      pixels[i]?logWrite("像素内存分配成功", 'I'):logWrite("内存分配失败", 'E');
  }
  
//嵌套循环遍历二维数组，期间绘图
for (short picY = 0; picY < imgHeight; picY++) 
{
    for (short picX = 0; picX < imgWidth; picX++) 
    {
      pixels[picX][picY].R = customColor_r;
      pixels[picX][picY].G = customColor_g;
      pixels[picX][picY].B = customColor_b;
      pixels[picX][picY].A = 0;
      //画竖线
      if (picX==imgWidth/2&&picY>imgHeight/2) 
        pixels[picX][picY].A = customColor_a;
      //画标尺
      
      if (data[scalesNumber].distance%50==0) //绘制50m倍数的标尺
      {
       
        if (data[scalesNumber].distance%100==0) //绘制100m倍数的标尺
        {
          if (picX>imgWidth/2&&picX<=imgWidth/2+customScaleLength_meets100&&picY>=imgHeight/2+data[scalesNumber].drop&&picY<imgHeight/2+data[scalesNumber].drop+customScaleHeight_meets100) 
          {
            pixels[picX][picY].A = customColor_a;
          //printf("current Y : %d, scale: %d, drop should be:%d\n",picY, data[scalesNumber].distance,data[scalesNumber].drop);
            if(picX==imgWidth/2+customScaleLength_meets100&&picY==imgHeight/2+data[scalesNumber].drop+customScaleHeight_meets100-1)//整条标尺绘制完后再前往下一个
              scalesNumber++;
          }
        }
        else//绘制50m倍数处标尺
        {
          if (picX>imgWidth/2&&picX<=imgWidth/2+customScaleLength_meets50&&picY>=imgHeight/2+data[scalesNumber].drop&&picY<imgHeight/2+data[scalesNumber].drop+customScaleHeight_meets50) 
          {
            pixels[picX][picY].A = customColor_a;
            //printf("current Y : %d, scale: %d, drop should be:%d\n",picY, data[scalesNumber].distance,data[scalesNumber].drop);
            if(picX==imgWidth/2+customScaleLength_meets50&&picY==imgHeight/2+data[scalesNumber].drop+customScaleHeight_meets50-1)
              scalesNumber++;
          }
        }
      }
      else//绘制25m倍数处标尺 
      {
        
        if (picX>imgWidth/2&&picX<=imgWidth/2+customScaleLength_normal&&picY>=imgHeight/2+data[scalesNumber].drop&&picY<imgHeight/2+data[scalesNumber].drop+customScaleHeight_normal) 
          {
            pixels[picX][picY].A = customColor_a;
            //printf("current Y : %d, scale: %d, drop should be:%d\n",picY, data[scalesNumber].distance,data[scalesNumber].drop);
            if(picX==imgWidth/2+customScaleLength_normal&&picY==imgHeight/2+data[scalesNumber].drop+customScaleHeight_normal-1)//整条标尺绘制完后再前往下一个
              scalesNumber++;

          }
      }
    }
}
  
  img=pixelsToArray( pixels);
  stbi_write_png("./OUTPUT.png", 1080, 1080, 4, img, 1080*4);
  //free子函数传回来的用完的一维数组
  free(img);
  //printf("%d\n",scalesNumber);
}

 bool fillData(scale * dataToDraw, const bulletState* bullet, unsigned short* targetX);

void returnToDefault(void); 
/*__declspec(dllexport) */int main(void) {
  logWrite("已启动", 'I');
  scale *dataToDraw = malloc(sizeof(scale)*((endAt-startFrom)/gap+1));   //传给绘制函数的要绘制的弹道数据, 由起始距离和间距决定数据点个数，动态分配内存 
  bulletState bullet={x,y,vx,vy,drag,0};
  
  
  
  if (zeroInExists==true) 
  {
    double FinalAngle = stimulation(&bullet);
    bullet.vy = -bullet.vx * sin(FinalAngle / 180 * M_PI); // 弹道初速度
    bullet.vx = bullet.vx * cos(FinalAngle / 180 * M_PI); // 弹道初速度
    
  }

    for (unsigned short targetX=startFrom,frame=0; mTime < 10; mTime += tick,frame++) {
        
        flush(&bullet);
        fillData(dataToDraw, &bullet, &targetX);

        
    }

  outputImg(dataToDraw);

  free(dataToDraw);
  
}


//重置用户自定义参数为默认值
void returnToDefault(void) {
  x=X;
  y=Y;
  vx=VX;
  vy=VY;
  drag=DRAG;
  tick=TICK;
  tolerance=TOLERANCE;
  startFrom=START_FROM;
  endAt=END_AT;
  gap=GAP;
  zeroInExists=ZERO_IN_EXISTS;
  customColor_r=DEAFALTCOLOR_R;
  customColor_g=DEAFALTCOLOR_G;
  customColor_b=DEAFALTCOLOR_B;
  customColor_a=(char)DEAFALTCOLOR_A;
  customScaleLength_normal=DEFAULTSCALELENGTH_NORMAL;
  customScaleHeight_normal=DEFAULTSCALEHEIGHT_NORMAL;
  customScaleLength_meets50=DEFAULTSCALELENGTH_MEETS50;
  customScaleHeight_meets50=DEFAULTSCALEHEIGHT_MEETS50;
  customScaleLength_meets100=DEFAULTSCALELENGTH_MEETS100;
  customScaleHeight_meets100=DEFAULTSCALEHEIGHT_MEETS100;
  zeroInDistance=ZERO_IN_DISTANCE;
  imgHeight=IMGHEIGHT;
  imgWidth=IMGWIDTH;
  enlargement=ENLARGEMENT;
  fov=FOV;
  screenHeight=SCREEN_HEIGHT;
  mTime=TIME;
  

}
 bool fillData(scale * dataToDraw, const bulletState* bullet, unsigned short* targetX) {
 
  static double lastDrop=0;
  static double dropNow=0.1; //上一次弹道下坠距离，本次弹道下坠距离
  static bulletState  temp[3]={0};//一个小队列，存储最近三次子弹状态
  temp[0] = temp[1];
  temp[1] = temp[2];
  temp[2] = *bullet;

  dropNow=closestIn3(temp, targetX);
  
  
  if(lastDrop==dropNow&&*targetX<=endAt)
  {
    dataToDraw[(*targetX-startFrom)/gap].drop=(short)round(dropNow);
    dataToDraw[(*targetX-startFrom)/gap].distance=*targetX;
    *targetX+=gap;
    return true;
  }

  lastDrop=dropNow;
  return false;
}
//三个中最接近给定距离的弹道数据
double closestIn3(const bulletState* bullet,const unsigned short* targetX) {
  double drop=0;
  if(fabs(bullet[0].x-*targetX)>=fabs(bullet[1].x-*targetX)) 
    if(fabs(bullet[1].x-*targetX)>=fabs(bullet[2].x-*targetX))
      drop=bullet[2].dropPixel;
    else drop=bullet[1].dropPixel;
  else drop=bullet[0].dropPixel;
  
  return drop;
}
char* pixelsToArray ( pixel **array2d) {
  char* array1d = (char*)malloc(sizeof(char)*imgWidth*imgHeight*4);
  for (short imgY = 0; imgY < imgHeight; imgY++) {
        for (short imgX = 0; imgX < imgWidth; imgX++) {
            int idx = (imgY * imgWidth + imgX) * 4;
            array1d[idx + 0] = array2d[imgX][imgY].R;
            array1d[idx + 1] = array2d[imgX][imgY].G;
            array1d[idx + 2] = array2d[imgX][imgY].B;
            array1d[idx + 3] = array2d[imgX][imgY].A;
        }
    }
  //free传下来的二维数组
  for (short i = 0; i < imgWidth; i++) 
    free(array2d[i]);
  
  free(array2d);
  return array1d;
}

void logWrite(const char *message, const char logLevel) {
  log_set_quiet(true);
  FILE* fp = fopen("./log.txt", "a+");
  log_add_fp(fp, LOG_TRACE);
  switch (logLevel) {
  case 'I':
    log_info(message);
    break;
  case 'W':
    log_warn(message);
    break;
  case 'E':
    log_error(message);
    break;
  default:
    log_info(message);
    break;
  }
}

bool stb_DrawLine(pixel **img, int x0, int y0, int x1, int y1, pixel color) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
  int err = dx + dy, e2; /* error value e_xy */
  while (true) {
    img[x0][y0] = color;
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}
void reset(bulletState* bullet) {
  bullet->x = x;
  bullet->y = y;
  bullet->vx = vx;
  bullet->vy = vy;
  bullet->drag = drag;
  bullet->dropPixel = 0;
}

bulletState* flush(bulletState* bullet) {
  
  // 先更新位置
  bullet->x += bullet->vx * tick;
  bullet->y += bullet->vy * tick;
  // 更新速度：先施加阻力，再施加重力
  double currentV=sqrt((bullet->vx*bullet->vx) + (bullet->vy*bullet->vy));
  bullet->vx *= 1 - (currentV * currentV * drag * tick) / currentV;
  bullet->vy *= 1 - (currentV * currentV * drag * tick) / currentV;
  bullet->vy += 12 * tick;  // 重力加速度
  // 计算下坠像素（保持不变）
  double screenh = tan((double)fov / 2 * M_PI / 180) * 2 * bullet->x;
    if (screenh != 0) {
        bullet->dropPixel = fabs(bullet->y / screenh) * screenHeight * enlargement;  // 取绝对值
    } else {
        bullet->dropPixel = 0;  // 避免除以 0
    }
  return bullet;
}