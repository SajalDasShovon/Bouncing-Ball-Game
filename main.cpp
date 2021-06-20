#include <windows.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <cmath>
#include <bits/stdc++.h>
#include <vector>
#include <iostream>
#include "include/BmpLoader.h"

#define PI 3.1416

//int font=(int)GLUT_BITMAP_8_BY_13;
// zz is font position x_look is side position
unsigned int ID;
//BmpLoader bl;

static float zz = 10;
static float x_look = 0;

static float score = 0;
static float final_score = 0;
float f=0;
GLfloat alpha = 0.0, theta = 0.0, axis_x=0.0, axis_y=0.0;
double Txval=0,Tyval=0,Tzval=0;
double moving=0;

int anglex= 0, angley = 0, anglez = 0;          //rotation angles
int window;
int wired=0;
int shcpt=1;
int animat = 0;
const int L=20;
const int dgre=3;
int ncpt=L+1;
int clikd=0;
const int nt = 40;				//number of slices along x-direction
const int ntheta = 5;


GLfloat ctrlpoints[L+1][3] =
{
    { 5.5, 1.7, 0.0}, { 6.8, 1.175, 0.0},
    { 6.55, -0.425, 0.0},{ 4.825, -1.325, 0.0},
    {4.075, 0.025, 0.0}, {4.65, 1.45, 0.0},
};


double ex=0, ey=0, ez=15, lx=0,ly=0,lz=0, hx=0,hy=1,hz=0;

float wcsClkDn[3],wcsClkUp[3];
///////////////////////////////
class point1
{
public:
    point1()
    {
        x=0;
        y=0;
    }
    int x;
    int y;
} clkpt[2];
int flag=0;
GLint viewport[4]; //var to hold the viewport info
GLdouble modelview[16]; //var to hold the modelview info
GLdouble projection[16]; //var to hold the projection matrix info

//////////////////////////
void scsToWcs(float sx,float sy, float wcsv[3] );
void processMouse(int button, int state, int x, int y);
void matColor(float kdr, float kdg, float kdb,  float shiny, int frnt_Back=0, float ambFactor=1.0, float specFactor=1.0);
///////////////////////////

void scsToWcs(float sx,float sy, float wcsv[3] )
{

    GLfloat winX, winY, winZ; //variables to hold screen x,y,z coordinates
    GLdouble worldX, worldY, worldZ; //variables to hold world x,y,z coordinates

    //glGetDoublev( GL_MODELVIEW_MATRIX, modelview ); //get the modelview info
    glGetDoublev( GL_PROJECTION_MATRIX, projection ); //get the projection matrix info
    glGetIntegerv( GL_VIEWPORT, viewport ); //get the viewport info

    winX = sx;
    winY = (float)viewport[3] - (float)sy;
    winZ = 0;

    //get the world coordinates from the screen coordinates
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &worldX, &worldY, &worldZ);
    wcsv[0]=worldX;
    wcsv[1]=worldY;
    wcsv[2]=worldZ;


}
void processMouse(int button, int state, int x, int y)
{
    if(button==GLUT_LEFT_BUTTON && state==GLUT_DOWN)
    {
        if(flag!=1)
        {
            flag=1;
            clkpt[0].x=x;
            clkpt[0].y=y;
        }


        scsToWcs(clkpt[0].x,clkpt[0].y,wcsClkDn);
        //cout<<"\nD: "<<x<<" "<<y<<" wcs: "<<wcsClkDn[0]<<" "<<wcsClkDn[1];
    }
    else if(button==GLUT_LEFT_BUTTON && state==GLUT_UP)
    {
        if (flag==1)
        {
            clkpt[1].x=x;
            clkpt[1].y=y;
            flag=0;
        }
        float wcs[3];
        scsToWcs(clkpt[1].x,clkpt[1].y,wcsClkUp);
        //cout<<"\nU: "<<x<<" "<<y<<" wcs: "<<wcsClkUp[0]<<" "<<wcsClkUp[1];

        clikd=!clikd;
    }
}

//control points
long long nCr(int n, int r)
{
    if(r > n / 2) r = n - r; // because C(n, r) == C(n, n - r)
    long long ans = 1;
    int i;

    for(i = 1; i <= r; i++)
    {
        ans *= n - r + i;
        ans /= i;
    }

    return ans;
}

//polynomial interpretation for N points
void BezierCurve ( double t,  float xy[2])
{
    double y=0;
    double x=0;
    t=t>1.0?1.0:t;
    for(int i=0; i<=L; i++)
    {
        int ncr=nCr(L,i);
        double oneMinusTpow=pow(1-t,double(L-i));
        double tPow=pow(t,double(i));
        double coef=oneMinusTpow*tPow*ncr;
        x+=coef*ctrlpoints[i][0];
        y+=coef*ctrlpoints[i][1];

    }
    xy[0] = float(x);
    xy[1] = float(y);

    //return y;
}

///////////////////////
void setNormal(GLfloat x1, GLfloat y1,GLfloat z1, GLfloat x2, GLfloat y2,GLfloat z2, GLfloat x3, GLfloat y3,GLfloat z3)
{
    GLfloat Ux, Uy, Uz, Vx, Vy, Vz, Nx, Ny, Nz;

    Ux = x2-x1;
    Uy = y2-y1;
    Uz = z2-z1;

    Vx = x3-x1;
    Vy = y3-y1;
    Vz = z3-z1;

    Nx = Uy*Vz - Uz*Vy;
    Ny = Uz*Vx - Ux*Vz;
    Nz = Ux*Vy - Uy*Vx;

    glNormal3f(-Nx,-Ny,-Nz);
}

void bottleBezier()
{
    int i, j;
    float x, y, z, r;				//current coordinates
    float x1, y1, z1, r1;			//next coordinates
    float theta;

    const float startx = 0, endx = ctrlpoints[L][0];
    //number of angular slices
    const float dx = (endx - startx) / nt;	//x step size
    const float dtheta = 2*PI / ntheta;		//angular step size

    float t=0;
    float dt=1.0/nt;
    float xy[2];
    BezierCurve( t,  xy);
    x = xy[0];
    r = xy[1];
    //rotate about z-axis
    float p1x,p1y,p1z,p2x,p2y,p2z;
    for ( i = 0; i < nt; ++i )  			//step through x
    {
        theta = 0;
        t+=dt;
        BezierCurve( t,  xy);
        x1 = xy[0];
        r1 = xy[1];

        //draw the surface composed of quadrilaterals by sweeping theta
        glBegin( GL_QUAD_STRIP );
        for ( j = 0; j <= ntheta; ++j )
        {
            theta += dtheta;
            double cosa = cos( theta );
            double sina = sin ( theta );
            y = r * cosa;
            y1 = r1 * cosa;	//current and next y
            z = r * sina;
            z1 = r1 * sina;	//current and next z

            //edge from point at x to point at next x
            glVertex3f (x, y, z);

            if(j>0)
            {
                setNormal(p1x,p1y,p1z,p2x,p2y,p2z,x, y, z);
            }
            else
            {
                p1x=x;
                p1y=y;
                p1z=z;
                p2x=x1;
                p2y=y1;
                p2z=z1;

            }
            glVertex3f (x1, y1, z1);

            //forms quad with next pair of points with incremented theta value
        }
        glEnd();
        x = x1;
        r = r1;
    } //for i

}


/*static void resize(int width, int height)
{
    const float ar = (float) width / (float) height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);
    // gluPerspective(60,ar, 2.0, 100.0);

    glOrtho(-8.0, 8.0, -8.0*(GLfloat)height/(GLfloat)width, 8.0*(GLfloat)height/(GLfloat)width, 2.0, 25.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
    gluLookAt(ex,ey,ez,lx,ly,lz,hx,hy,hz);

}
*/


void chata()
{
      const double t = glutGet(GLUT_ELAPSED_TIME) / 5000.0;
    const double a = t*90.0;

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(wired)
    {
        glPolygonMode( GL_FRONT, GL_LINE ) ;
        glPolygonMode( GL_BACK, GL_LINE ) ;

    }
    else
    {
        glPolygonMode( GL_FRONT,GL_FILL ) ;
        glPolygonMode( GL_BACK, GL_FILL ) ;
    }

    glPushMatrix();

    if(animat)
        glRotated(a,0,0,1);

    glRotatef( anglex, 1.0, 0.0, 0.0);
    glRotatef( angley, 0.0, 1.0, 0.0);         	//rotate about y-axis
    glRotatef( anglez, 0.0, 0.0, 1.0);

    glRotatef( 90, 0.0, 0.0, 1.0);
    glTranslated(-6,-6,-14);
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview ); //get the modelview info

    matColor(0.9,0.5,0.1,20);   // front face color
    matColor(0.0,0.5,0.8,20,1);  // back face color


    bottleBezier();


    if(shcpt)
    {
        matColor(0.0,0.0,0.9,20);
        //showControlPoints();
    }

    glPopMatrix();
}
void showControlPoints()
{
    glPointSize(5.0);
    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_POINTS);
    for (int i = 0; i <=L; i++)
        glVertex3fv(&ctrlpoints[i][0]);
    glEnd();
}
static void idle(void)
{
    glutPostRedisplay();
}

const GLfloat light_ambient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 1.0f };

void matColor(float kdr, float kdg, float kdb,  float shiny, int frnt_Back, float ambFactor, float specFactor)
{

    const GLfloat mat_ambient[]    = { kdr*ambFactor, kdg*ambFactor, kdb*ambFactor, 1.0f };
    const GLfloat mat_diffuse[]    = { kdr, kdg, kdb, 1.0f };
    const GLfloat mat_specular[]   = { 1.0f*specFactor, 1.0f*specFactor, 1.0f*specFactor, 1.0f };
    const GLfloat high_shininess[] = { shiny };
    if(frnt_Back==0)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    }
    else if(frnt_Back==1)
    {
        glMaterialfv(GL_BACK, GL_AMBIENT,   mat_ambient);
        glMaterialfv(GL_BACK, GL_DIFFUSE,   mat_diffuse);
        glMaterialfv(GL_BACK, GL_SPECULAR,  mat_specular);
        glMaterialfv(GL_BACK, GL_SHININESS, high_shininess);
    }
    else if(frnt_Back==2)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mat_ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, high_shininess);
    }

}

/* Program entry point */
void myInit()
{
    glClearColor(.1,.1,.1,1);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
   // glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

static void resize(int width, int height)
{
    const float ar = (float) width / (float) height;
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-ar, ar, -1.0, 1.0, 2.0, 1000.0);
    //glOrtho (0.0, width, 0.0, height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

static GLfloat v_cube[8][3] =
{
    {0,0,0},
    {0,0,1},
    {0,1,0},
    {0,1,1},
    {1,0,0},
    {1,0,1},
    {1,1,0},
    {1,1,1}
};

static GLubyte c_ind[6][4] =
{
    {3,1,5,7},
    {2,0,1,3},
    {7,5,4,6},
    {2,3,7,6},
    {1,0,4,5},
    {6,4,0,2}
};


static void getNormal3p(GLfloat x1, GLfloat y1, GLfloat z1, GLfloat x2, GLfloat y2, GLfloat z2, GLfloat x3, GLfloat y3, GLfloat z3)
{
    GLfloat Ux, Uy, Uz, Vx, Vy, Vz, Nx, Ny, Nz;

    Ux = x2-x1;
    Uy = y2-y1;
    Uz = z2-z1;

    Vx = x3-x1;
    Vy = y3-y1;
    Vz = z3-z1;

    Nx = Uy*Vz - Uz*Vy;
    Ny = Uz*Vx - Ux*Vz;
    Nz = Ux*Vy - Uy*Vx;

    glNormal3f(Nx,Ny,Nz);
}
void drawcube(float cr, float cg, float cb,int n=1, bool e=false)
{
    GLfloat m_no[] = {0, 1, 0, 1.0};
    GLfloat m_amb[] = {cr,cg,cb,1};
    GLfloat m_diff[] = {cr,cg,cb,1};
    GLfloat m_spec[] = {1,0,0,1};
    GLfloat m_sh[] = {30};

    GLfloat m_em[] = {1,0,0,1};

    glMaterialfv(GL_FRONT, GL_AMBIENT, m_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, m_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, m_spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, m_sh);



    glBegin(GL_QUADS);
    for(GLint i = 0; i<6; i++)
    {
        //glColor3f(cr,cg,cb);
        getNormal3p(v_cube[c_ind[i][0]][0], v_cube[c_ind[i][0]][1], v_cube[c_ind[i][0]][2],
                    v_cube[c_ind[i][1]][0], v_cube[c_ind[i][1]][1], v_cube[c_ind[i][1]][2],
                    v_cube[c_ind[i][2]][0], v_cube[c_ind[i][2]][1], v_cube[c_ind[i][2]][2]);

        glVertex3fv(&v_cube[c_ind[i][0]][0]);
        glTexCoord2f(0,0);
        glVertex3fv(&v_cube[c_ind[i][1]][0]);
        glTexCoord2f(1,0);
        glVertex3fv(&v_cube[c_ind[i][2]][0]);
        glTexCoord2f(1,1);
        glVertex3fv(&v_cube[c_ind[i][3]][0]);
        glTexCoord2f(0,1);
    }
    glEnd();
}



void reff(void)
{
    glPushMatrix();
    glScaled(1,20,1);
    drawcube(0,1,0);
    glPopMatrix();
}
void light()
{


    GLfloat l_no[] = {0, 0, 0, 1.0};
    GLfloat l_amb[] = {0.1, 0.1, 0.1, 1.0};
    GLfloat l_dif[] = {1,1,1,1};
    GLfloat l_spec[] = {0.2,0.2,0.2,1};
    GLfloat l_pos1[] = {0,1,0,1.0};
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0,GL_AMBIENT,l_amb);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,l_dif);
    glLightfv(GL_LIGHT0,GL_SPECULAR,l_spec);


}


void axis(void)
{

    glBegin(GL_LINES);
    glColor3f (1.0, 0.0, 0.0); ///red is X
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(50.0, 0.0, 0.0);

    glColor3f (0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 50.0, 0.0); /// green is Y

    glColor3f (0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 50.0); ///blue is Z
    glEnd();

}

void Floor(void)
{
    for(int i=1; i<18; i++)
    {

         glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,3);
        glPushMatrix();
        glTranslatef(-100,-3,i*-100);
        glScaled(200,1,110);
        drawcube(.322,.745,.5);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);

    }
}


void track(void)
{


    ///road
    for(int i=1; i<600; i++)
    {


        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,2);

        glPushMatrix();
        glTranslatef(-7.5,-2.9,-10*i);
        //reff();
        glScaled(17,1,10);
        drawcube(1,0,0,1);
        glPopMatrix();

        glDisable(GL_TEXTURE_2D);
    }


}
void output(int x, int y, float r, float g, float b, char *string)
{
  glColor3f( r, g, b );
  glOrtho(0,800,0,600,-5,5);
  glMatrixMode(GL_MODELVIEW);
  glRasterPos2i(x, y);
  int len, i;
  len = (int)strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24 , string[i]);
  }
  return;
}

void text(float f=0)
{
    glPushMatrix();

    glTranslatef(-3,0,0+f);
char str[80];
   sprintf(str, "Your Score = %d", score);
   puts(str);
output(0,0,1.0,1.0,0,str);
    glPopMatrix();
}

void building(void)
{
    /// building
    for(int i = 1; i<18; i++)
    {
        //srand(i);

        int rand1,rand2,rand3,rand4;
        srand(i);
        rand1 = (rand() % 13) + 7;
        srand(i+1);
        rand2 = (rand() % 13) + 7;
        srand(i+2);
        rand3 = (rand() % 13) + 7;
        srand(i+3);
        rand4 = (rand() % 13) + 17;
        //printf("%d :rand1 %d rand2 %d rand3 \n",rand1,rand2,rand3);

        /// buildings
         glEnable(GL_TEXTURE_2D);
         glBindTexture(GL_TEXTURE_2D,5);
        glPushMatrix();
        glTranslatef(-25,-4,-i*100);
        glScaled(10,rand4,10);
        drawcube(255.0/255.0F,0.0F,0.0F);
        glPopMatrix();
        //glDisable(GL_TEXTURE_2D);

        glPushMatrix();
        glTranslatef(-25,-4,(-i*100)+25);
        glScaled(10,rand2,10);
        drawcube(0.0F,255.0/255.0F,0.0F);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-25,-4,(-i*100)+50);
        glScaled(10,rand1,10);
        drawcube(0.0F,0.0F,255.0/255.0F);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-25,-4,(-i*100)+75);
        glScaled(10,rand3,10);
        drawcube(255.0/255.0F,255.0/255.0F,0.0F);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-25,-4,-i*100);
        glScaled(10,rand4,10);
        drawcube(255.0/255.0F,0.0F,0.0F);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(15,-4,(-i*100)+25);
        glScaled(10,rand2,10);
        drawcube(0.0F,255.0/255.0F,0.0F);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(15,-4,(-i*100)+50);
        glScaled(10,rand1,10);
        drawcube(0.0F,0.0F,255.0/255.0F);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(15,-4,(-i*100)+75);
        glScaled(10,rand3,10);
        drawcube(255.0/255.0F,255.0/255.0F,0.0F);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);

    }
}


void Ball(){
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    double a = t*90.0;

    glColor3d(0,1,0);


    glPushMatrix();
        glTranslated(0,0,0);
        glScaled(3,3,3);
        glutSolidSphere(1,50,50);
     glPopMatrix();
      glColor3d(0,0,0);
    glPushMatrix();
        glTranslated(1.7,0.1,0);
        glScaled(1.5,0.7,0.8);
        glRotated(40,0,1,0);
        glutSolidSphere(0.45,30,30);

    glPopMatrix();



}


void sphere()
{
    //glTranslated(3,0,3);
    // GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient[] = { 0, 0, 1, 1.0 };
    GLfloat mat_diffuse[] = { 0, 0, 1, 1.0 };
    GLfloat mat_specular[] = {0, 0, 1 , 1.0 };
    GLfloat mat_shininess[] = {2};

    glMaterialfv( GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv( GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess);
    glutSolidSphere (1.0, 20, 16);
}
void cylinderkata()
{

    // glTranslated(3,0,3);
    // GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient[] = { 1, 0, 1, 1.0 };
    GLfloat mat_diffuse[] = { 1, 0, 1, 1.0 };
    GLfloat mat_specular[] = { 1,1,1, 1.0 };
    GLfloat mat_shininess[] = {10};

    glMaterialfv( GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv( GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess);


    glPushMatrix();
    //glColor3d(1,0,0);
    GLUquadricObj *quadratic;
    quadratic = gluNewQuadric();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadratic,0.5f,0.5f,3.0f,32,32);

    ///kata on cylinder
    glPushMatrix();
    glRotatef(-40,0,0,1);
    glTranslatef(-1,0,0);
    glScaled(2,.1,0.1);
    drawcube(1,0,0);
    glPopMatrix();
    ///kata on cylinder
    glPushMatrix();
    glRotatef(40,0,0,1);
    glTranslatef(-1,0,0.5);
    glScaled(2,.1,0.1);
    drawcube(1,0,0);
    glPopMatrix();
    ///kata on cylinder
    glPushMatrix();
    glRotatef(-80,0,0,1);
    glTranslatef(-1,0,1.0);
    glScaled(2,.1,0.1);
    drawcube(1,0,0);
    glPopMatrix();
    ///kata on cylinder
    glPushMatrix();
    glRotatef(-30,0,0,1);
    glTranslatef(-1,0,1.5);
    glScaled(2,.1,0.1);
    drawcube(1,0,0);
    glPopMatrix();
    ///kata on cylinder
    glPushMatrix();
    glRotatef(60,0,0,1);
    glTranslatef(-1,0,2.0);
    glScaled(2,.1,0.1);
    drawcube(1,0,0);
    glPopMatrix();
    ///kata on cylinder
    glPushMatrix();
    glRotatef(100,0,0,1);
    glTranslatef(-1,0,2.5);
    glScaled(2,.1,0.1);
    drawcube(1,0,0);
    glPopMatrix();





    glPopMatrix();
    //glutSolidSphere (1.0, 20, 16);

}
float rotat;
static void display(void)
{

    // axis();
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    const double a = t*90.0;
    glClearColor(0.5, 0.8, 0.9 , 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /// GLUE LOOK AT
    glLoadIdentity();
    if (zz>-850)
        zz = zz - 0.0;
    if(zz<-850)
        zz=-50;
    //printf("%0.2lf \n",zz);
    //if (zz>-748)
    //      score = score + .1;
//   final_score = score* 200 / 745;

    //gluLookAt(60+ x_look, 10.0, -100 + zz,
      //        -200.0+ x_look, -5.0, -100.0 + zz,
        //      0.0, 1.0, 0.0);

         gluLookAt(0.0+ x_look, 3.0, 7.5 + zz,
             -0.5 + x_look, 1.0, -10.0 + zz,
              0.0, 1.0, 0.0);

              glPushMatrix();

        glTranslatef(-0.2+x_look,-1,-0.2+zz);

    glRotatef(rotat,1,0,0);
    //chata();

    sphere();

    glPopMatrix();




    text(f);

    //sphere();
    //light();

    /*
    glPushMatrix();
    glRotatef(-90,1,0,0);
    glTranslatef(0,0,0);
    glScaled(2,0.3,3);
    drawcube(1,0,0);
    glPopMatrix();

    glPushMatrix();
    glRotatef(-60,1,0,0);
    glTranslatef(2,0,0);
    glScaled(2,0.3,3);
    drawcube(1,0,1);
    glPopMatrix();


*/

    Floor();
    track();
    building();
    //Ball();

    //car();
    //axis();
    glPushMatrix();
    glLoadIdentity();


    ///write the string
    //glTranslated(-.3+x_look,1.6+5,-8+zz);
    //drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();



    ///collusion detection
    ///Texture cube
    ///obstacle no 1
    int slide=0,look=0;
    slide = 80;
    look = 3;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,4);
    glPushMatrix();
    glTranslatef(-7.4+look,-2.3,-5-slide);

    glPushMatrix();
    glScaled(5,3,1);
    drawcube(255.1/255,255.1/255,53.1/255);
    //sphere();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);


    /* glScaled(2,1,0);
    //drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();



    glPushMatrix();
    glTranslatef(1.4 + look,-2.3,-5- slide);
    glPushMatrix();
    glScaled(1,10,1);
    //drawcube(0,1,0);
    glPopMatrix();
    glScaled(4,1,2);
    //drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();
    */


   if (zz>-6.5 - slide&& zz<-2.3- slide)
    {
        if(x_look <1.9 || x_look > 6)
        {
            zz = zz + 40;
            score = score + 10;
            if(score == 0)
            {
                //score+=50;
                printf("Game Over \n");
            }
            else
            {
                printf(" Score is %d \n",score);

            }

            //printf("%lf %lf %lf \n",x_look,-6.5 - slide,-2.3- slide);
        }

    }



    ///obstacle no 2
    slide = 100;
    look = 9;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,1);
    glPushMatrix();
    //glBindTexture(GL_TEXTURE_2D,2);
    // glEnable(GL_TEXTURE_2D);
    glTranslatef(-5.4 + look,-2.3,-5-slide);
    glScaled(8,5,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);




    glPushMatrix();
    glTranslatef(1.4 +2 + look,-2.3,-5- slide);
    glScaled(8,1,2);
    //drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();
    /*

    if (zz>-6.7 - slide&& zz<-2.3- slide)
    {
        if(x_look <-1.4 || x_look > -.6)
        {
            zz = zz + 40;
            score = score - 100;
        }
    }
    */
    /*
    ///obstacle no 3
    slide = 230;
    look = 10;
    glPushMatrix();
    // glBindTexture(GL_TEXTURE_2D,2);
    // glEnable(GL_TEXTURE_2D);
    glTranslatef(-5.4-2 + look,-2.3,-5-slide);
    glScaled(5,6,3);
    glutSolidSphere(1 ,6,7);
    //drawcube(255.1/255,255.1/255,53.1/255);
    sphere();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.4 + look,-2.3,-5- slide);
    glScaled(4,2,2);
    //drawcube(255.1/255,255.1/255,53.1/255);
    //glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    */
    /*
    if (zz>-6.7 - slide&& zz<-2.3- slide)
    {
        if(x_look <2.6 || x_look > 3.3)
        {
            zz = zz + 40;
            score = score - 100;
        }
    }

    */
    ///obstacle no 4 kata
    slide = 320;
    look = -3;
    glPushMatrix();
    //glBindTexture(GL_TEXTURE_2D,2);
    //glEnable(GL_TEXTURE_2D);
    glTranslatef(-5.4+2 +look+ moving,-2.3,-5-slide);
    //glScaled(1,1,5);

    glPushMatrix();


    //glScaled(1,1,10);
    glRotatef(rotat,0,1,0);
    glScaled(2,4,3);
    glPushMatrix();
    cylinderkata();
    glPopMatrix();


    glPushMatrix();
    //chata();
    glPopMatrix();
    /*

//   if (zz>-6.7 - slide&& zz<-2.3- slide)
//   {
    //      if(x_look <-1.4 || x_look > -.6)
    //      {
    //          zz = zz + 40;
    //         score = score - 100;
    //      }
    // }
    */

    ///obstacle no 5

    slide = -500;
    look = 0;
    glBindTexture(GL_TEXTURE_2D,7);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();

    glTranslatef(-5.4,-2.3,-5+ slide);
    glScaled(4,3,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.4,-2.3,-5+ slide);
    glScaled(4,3,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    /*
    if (zz>-507 && zz<-502)
    {
        if(x_look>.4 || x_look < -.4)
        {
            zz = zz + 40;
            score = score - 100;
        }
    }
    */

    ///obstacle no 6
    slide =-300 ;

    glBindTexture(GL_TEXTURE_2D,8);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslatef(-5.4,-2.3,-5 + slide);
    glScaled(3,7,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(1.4,-2.3,-5+ slide);
    glScaled(3,7,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    /*

    if (zz>-6.7+slide && zz<-2.3+slide)
    {
        if(x_look>.4 || x_look < -.4)
        {
            zz = zz + 40;
            score = score - 100;
        }
    }

    */
    ///obstacle no 7
    slide = 700;
    look = 3;
    glPushMatrix();
    glTranslatef(-7.4+look,-2.3,-5-slide);
    glScaled(6,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();



    glPushMatrix();
    glTranslatef(1.4 + look,-2.3,-5- slide);
    glScaled(4,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();


    ///obstacle no 8
    slide = 800;
    look = -3;
    glPushMatrix();
    //glBindTexture(GL_TEXTURE_2D,2);
    // glEnable(GL_TEXTURE_2D);
    glTranslatef(-5.4+2 + look,-2.3,-5-slide);
    //reff();
    glScaled(4,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.4 +2 + look,-2.3,-5- slide);
    glScaled(8,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();

    ///obstacle no 9-1
    slide = 900;
    look = 3;
    glPushMatrix();
    glTranslatef(-7.4+look,-2.3,-5-slide);

    glPushMatrix();
    glScaled(1,10,1);
    drawcube(0,1,0);
    glPopMatrix();


    glScaled(6,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();



    glPushMatrix();
    glTranslatef(1.4 + look,-2.3,-5- slide);
    glPushMatrix();
    glScaled(1,10,1);
    drawcube(0,1,0);
    glPopMatrix();
    glScaled(4,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();

    ///obstacle no 10-2
    slide = 1000;
    look = -3;
    glPushMatrix();
    //glBindTexture(GL_TEXTURE_2D,2);
    // glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D,2);
    // glEnable(GL_TEXTURE_2D);
    glTranslatef(-5.4+2 + look,-2.3,-5-slide);
    glScaled(4,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();



    glPushMatrix();
    glTranslatef(1.4 +2 + look,-2.3,-5- slide);
    glScaled(8,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();


    ///obstacle no 11-3

    slide = -1100;
    look = 0;
    glPushMatrix();
    // glBindTexture(GL_TEXTURE_2D,2);
    // glEnable(GL_TEXTURE_2D);
    glTranslatef(-5.4-2 + look,-2.3,-5-slide);
    glScaled(6,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.4 + look,-2.3,-5- slide);
    glScaled(4,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    //glDisable(GL_TEXTURE_2D);
    glPopMatrix();


    ///obstacle no 12-4
    slide = 1200;
    look = -3;
    glPushMatrix();
    //glBindTexture(GL_TEXTURE_2D,2);
    //glEnable(GL_TEXTURE_2D);
    glTranslatef(-5.4+2 + look,-2.3,-5-slide);
    glScaled(4,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();




    glPushMatrix();
    glTranslatef(1.4 +2 + look,-2.3,-5- slide);
    glScaled(8,1,2);
    drawcube(255.1/255,255.1/255,53.1/255);
    glPopMatrix();
    glPushMatrix();
    chata();
    glPopMatrix();
    glutSwapBuffers();




}

static void key(unsigned char key, int x, int y)
{

    switch (key)
    {
    case 27 :
    case 'q':
        exit(0);
        break;

    case 'i':
        zz = zz - 1.1;
        f=f-1.1;
        break;
    case 'k':
        zz = zz + 1.1;
        f=f+1.1;
        break;

    case 'l':
        if ( x_look < 5)
            x_look = x_look + 0.2;
        else
            x_look = x_look;
        break;
    case 'j':
        if ( x_look > -5)
            x_look = x_look - 0.2;
        else
            x_look = x_look;
        break;
        break;
    case 'r':

         case 'A':
        animat=!animat;
        break;

    case 's':
    case 'S':
        shcpt=!shcpt;
        break;

    case 'w':
    case 'W':
        wired=!wired;
        break;

    case 'x':
        anglex = ( anglex + 3 ) % 360;
        break;
    case 'X':
        anglex = ( anglex - 3 ) % 360;
        break;

    case 'y':
        angley = ( angley + 3 ) % 360;
        break;
    case 'Y':
        angley = ( angley - 3 ) % 360;
        break;

    case 'z':
        anglez = ( anglez + 3 ) % 360;
        break;
    case 'Z':
        anglez = ( anglez - 3 ) % 360;
        break;

    case 'b':
        light();
        break;
    case 'B':
        light();
        break;

        int c, d;

        for (c = 1; c <= 32767; c++)
            for (d = 1; d <= 32767; d++)
            {}
        zz = 0;
        x_look = 0;
        score = 0;
        final_score = 0;
        break;
    }

    glutPostRedisplay();
}

//bool flag=true;
/*static void idle(void)
{



    rotat+=10;
    if(rotat > 360)
        rotat = 0;

    moving+=0.020;
    if(moving>=15)
    {
        moving=0;
        // flag=false;
    }

    //printf("%lf\n",moving);
    //std::cout << flag << std::endl;

    glutPostRedisplay();
}
*/
void LoadTexture(const char*filename)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    BmpLoader bl(filename);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, bl.iWidth, bl.iHeight, GL_RGB, GL_UNSIGNED_BYTE, bl.textureData );
}



int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(640,480);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition (100, 100);





    printf("########## Press A for LEFT########## \n");
    printf("########## Press D for RIGHT ##########\n");
    printf("########## Press W for ZOOM IN ##########\n");
    printf("########## Press S for ZOOM OUT ##########\n");
    printf("Try to avoid the OBSTACLE\n \nTRY FOR YOUR BEST\n\n\n\n\n");

    glutCreateWindow("project-1607030");

    LoadTexture("C:\\Users\\Sajal Shovon\\Downloads\\Texture-20210603T181215Z-001\\Texture\\brick.bmp");
    LoadTexture("C:\\Users\\Sajal Shovon\\Desktop\\GL project\\project-1607030\\Traffic_Safety_44.bmp");
    LoadTexture("C:\\Users\\Sajal Shovon\\Desktop\\GL project\\project-1607030\\green-grass-texture-background-top-view-bright-grass-garden-idea-concept_109549-283.bmp");
    LoadTexture("C:\\Users\\Sajal Shovon\\Desktop\\GL project\\project-1607030\\uefa-champions.bmp");
    LoadTexture("C:\\Users\\Sajal Shovon\\Desktop\\GL project\\project-1607030\\221b_baker.bmp");
    LoadTexture("C:\\Users\\Sajal Shovon\\Desktop\\GL project\\project-1607030\\A1.bmp");
    LoadTexture("C:\\Users\\Sajal Shovon\\Desktop\\GL project\\project-1607030\\fire.bmp");
    LoadTexture("C:\\Users\\Sajal Shovon\\Desktop\\GL project\\project-1607030\\unnamed.bmp");
    glutReshapeFunc(resize);
    //glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    myInit();
    glutIdleFunc(idle);


    glShadeModel( GL_SMOOTH );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glEnable(GL_LIGHTING);


    glutMainLoop();

    return EXIT_SUCCESS;
}
