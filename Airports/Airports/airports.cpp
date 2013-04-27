//-----------------------------------------------------------------------------
//  [VIZ] US airports network
//  25/4/2013
//  Jan Volny (volnyja1@fel.cvut.cz), Jakub Vampola (vampojak@fel.cvut.cz)
//-----------------------------------------------------------------------------
//  Controls: 
//    [mouse-left-button]  ... scene rotation
//    [+], [-]             ... move scene forward/backward
//    [w]                  ... toggle wire mode
//    [c]                  ... toggle face culling
//-----------------------------------------------------------------------------
#define USE_ANTTWEAKBAR

#include <iostream>
#include <cstdlib>
#include "../common/common.h"
#include "../common/models/elephant.h"
#include "../common/models/cube.h"
#include "glut.h"
#include "../common/GLEW/glew.h"
#include "../common/GLFW/glfw.h"
#include "../IL/il.h"
#include "../IL/ilu.h"
#include "../IL/ilut.h"
#include "pugixml.hpp"
#include "Edge.h"

#define ILUT_USE_OPENGL

using namespace std;

// GLOBAL CONSTANTS____________________________________________________________
const char* VS_FILE_NAME     = "simple.vs";  // Vertex shader source file
const char* GS_FILE_NAME     = "simple.gs";  // Geometry shader source file
const char* FS_FILE_NAME     = "simple.fs";  // Fragment shader source file

// GLOBAL VARIABLES____________________________________________________________
GLint    g_WindowWidth       = 1366;    // Window width
GLint    g_WindowHeight      = 768;    // Window height

GLfloat  g_SceneRot[]        = { 0.0f, 0.0f, 0.0f, 1.0f }; // Scene orientation
GLfloat  g_SceneRot1[]       = { -5.5f, 1.0f, 0.0f, 0.0f }; // Scene orientation
GLfloat  g_SceneTraZ         = 120.0f;   // Scene translation along z-axis
GLfloat  g_SceneTraX         = 0.0f;   // Scene translation along z-axis
GLfloat  g_SceneTraY         = 0.0f;   // Scene translation along z-axis
GLfloat  g_RotSpeed          = 1.0f;   // Scene translation along z-axis
bool     g_SceneRotEnabled   = false;  // Scene auto-rotation enabled/disabled
bool     g_WireMode          = false;  // Wire mode enabled/disabled
bool     g_FaceCulling       = false;  // Face culling enabled/disabled

bool     g_UseShaders        = false;  // Programmable pipeline on/off
bool     g_UseVertexShader   = false;  // Use vertex shader
bool     g_UseGeometryShader = false;  // Use geometry shader
bool     g_UseFragmentShader = false;  // Use fragment shader

bool hide = true; // hide background map
bool antialiasing = false;
bool drawEdgesFlag = true;

enum EGeometry                         // Geometry type enum
{
	SPHERE_GEOMETRY,
	PLANE_GEOMETRY,
	NUM_GEOMETRY_TYPES
};
int g_GeometryType = PLANE_GEOMETRY; // Geometry type

// GLSL variables
GLuint g_ProgramId = 0;                 // Shader program id
GLuint g_TexId     = 0;
GLuint textureid;

int      g_TexMinFilter  = GL_NEAREST;
int      g_TexMagFilter  = GL_NEAREST;




// Airlines variables__________________________________________________________
vector<Airport> nodes;
vector<Edge> edges;
double avgX = 0.0;
double avgY = 0.0;
bool inverseLinearModel = true;
bool showPoints = false;
bool drawHeight = true;
bool drawColor = true;
float edgeWidth = 1.0;
double maxLengthOfEdge = 0.0;
float CeArray[2101][2101];

int debugCount = 2101;

// edge  bundling parametres
double K = 5.0;
double S = 0.15;
double compatibilityThreshold = 0.6;
int		iterations[5]	= {10,		10,		10,		5,		5};
double	Ss[5]			= {0.5,		0.15,	0.008,	0.004,	0.002};
double	Ks[5]			= {5.0,		5.0,	10.0,	20.0,	40.0};


//_____________________________________________________________________________




// FORWARD DECLARATIONS________________________________________________________
#ifdef USE_ANTTWEAKBAR
    void TW_CALL cbSetShaderStatus(const void*, void*);
    void TW_CALL cbGetShaderStatus(void*, void*);
#endif
void TW_CALL cbCompileShaderProgram(void *clientData);
void initGUI();

static GLfloat scene_rot = 0.0f;


GLuint textureID, textureID1;

int LoadGLTextures(){
	cout << "Texture creation..." << endl;
	ilInit();
	ILuint imageID;				// Create an image ID as a ULuint
	ILuint usaID;
	ILboolean success;			// Create a flag to keep track of success/failure
	ILenum error;				// Create a flag to keep track of the IL error state
	ilGenImages(1, &imageID); 		// Generate the image ID
	ilBindImage(imageID); 			// Bind the image
	string path = "../data/texture.jpg";
	success = ilLoadImage((const ILstring)path.c_str()); 	// Load the image file
 
	// If we managed to load the image, then we can start to do things with it...
	if (success)
	{
		// If the image is flipped (i.e. upside-down and mirrored, flip it the right way up!)
		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);
		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		{
			iluFlipImage();
		}
		// Convert the image into a suitable format to work with
		// NOTE: If your image contains alpha channel you can replace IL_RGB with IL_RGBA
		success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
		// Quit out if we failed the conversion
		if (!success)
		{
			error = ilGetError();
			std::cout << "Image conversion failed - IL reports error: " << error << " - " << iluErrorString(error) << std::endl;
			exit(-1);
		}

		glEnable(GL_TEXTURE_2D);
		// Generate a new texture
		glGenTextures(1, &textureID);
		// Bind the texture to a name
		glBindTexture(GL_TEXTURE_2D, textureID);
 
		// Set texture clamping method
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
 
		// Set texture interpolation method to use linear interpolation (no MIPMAPS)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Specify the texture specification
		glTexImage2D(GL_TEXTURE_2D, 				// Type of texture
						0,				// Pyramid level (for mip-mapping) - 0 is the top level
						ilGetInteger(IL_IMAGE_BPP),	// Image colour depth
						ilGetInteger(IL_IMAGE_WIDTH),	// Image width
						ilGetInteger(IL_IMAGE_HEIGHT),	// Image height
						0,				// Border width in pixels (can either be 1 or 0)
						ilGetInteger(IL_IMAGE_FORMAT),	// Image format (i.e. RGB, RGBA, BGR etc.)
						GL_UNSIGNED_BYTE,		// Image data type
						ilGetData());			// The actual image data itself
	}
  	else // If we failed to open the image file in the first place...
  	{
		error = ilGetError();
		cout << "Image load failed - IL reports error: " << error << " - " << iluErrorString(error) << endl;
		system("pause");
		exit(-1);
  	}

	ilGenImages(1, &usaID); 		// Generate the image ID
	ilBindImage(usaID); 			// Bind the image
	string path1 = "../data/usa_texture1.jpg";
	success = ilLoadImage((const ILstring)path1.c_str()); 	// Load the image file
 
	// If we managed to load the image, then we can start to do things with it...
	if (success)
	{
		// If the image is flipped (i.e. upside-down and mirrored, flip it the right way up!)
		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);
		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		{
			iluFlipImage();
		}
		// Convert the image into a suitable format to work with
		// NOTE: If your image contains alpha channel you can replace IL_RGB with IL_RGBA
		success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
		// Quit out if we failed the conversion
		if (!success)
		{
			error = ilGetError();
			std::cout << "Image conversion failed - IL reports error: " << error << " - " << iluErrorString(error) << std::endl;
			exit(-1);
		}

		glEnable(GL_TEXTURE_2D);
		// Generate a new texture
		glGenTextures(1, &textureID1);
		// Bind the texture to a name
		glBindTexture(GL_TEXTURE_2D, textureID1);
 
		// Set texture clamping method
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
 
		// Set texture interpolation method to use linear interpolation (no MIPMAPS)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Specify the texture specification
		glTexImage2D(GL_TEXTURE_2D, 				// Type of texture
						0,				// Pyramid level (for mip-mapping) - 0 is the top level
						ilGetInteger(IL_IMAGE_BPP),	// Image colour depth
						ilGetInteger(IL_IMAGE_WIDTH),	// Image width
						ilGetInteger(IL_IMAGE_HEIGHT),	// Image height
						0,				// Border width in pixels (can either be 1 or 0)
						ilGetInteger(IL_IMAGE_FORMAT),	// Image format (i.e. RGB, RGBA, BGR etc.)
						GL_UNSIGNED_BYTE,		// Image data type
						ilGetData());			// The actual image data itself
		
 	}
  	else // If we failed to open the image file in the first place...
  	{
		error = ilGetError();
		cout << "Image load failed - IL reports error: " << error << " - " << iluErrorString(error) << endl;
		system("pause");
		exit(-1);
  	}

 	//ilDeleteImages(1, &imageID); // Because we have already copied image data into texture data we can release memory used by image.
	cout << "Texture creation successful." << endl;
	return textureID; // Return the GLuint to the texture so you can use it!
}

double euclidDist(Point p, Point q){
	return sqrt((q.getX()-p.getX())*(q.getX()-p.getX())+(q.getY()-p.getY())*(q.getY()-p.getY())+(q.getZ()-p.getZ())*(q.getZ()-p.getZ()));
}

double angleCompatibility(Edge p, Edge q){
	/*double alpha = (p.getSource()->getX()*p.getTarget()->getX()+p.getSource()->getY()*p.getTarget()->getY())/(p.getLength()*q.getLength());
	double comp = abs(cos(alpha));
	if(comp > 1.0 || comp < 0.0){
		cout << "angle compatibility: " << comp << endl;
		system("pause");
	}
	return comp;*/
	return abs(cos((p.getSource()->getX()*p.getTarget()->getX()+p.getSource()->getY()*p.getTarget()->getY())/(p.getLength()*q.getLength())));
}

double scaleCompatibility(Edge p, Edge q){
	double avgLength = (p.getLength()/maxLengthOfEdge+q.getLength()/maxLengthOfEdge)/2;
	/*double comp = 1.0/(avgLength*min(p.getLength()/maxLengthOfEdge,q.getLength()/maxLengthOfEdge)+
					   max(p.getLength()/maxLengthOfEdge,q.getLength()/maxLengthOfEdge)/avgLength);
	if(comp > 1.0 || comp < 0.0){
		cout << "scale compatibility: " << comp << endl;
		system("pause");
	}
	return comp;*/
	return 1.0/(avgLength*min(p.getLength()/maxLengthOfEdge,q.getLength()/maxLengthOfEdge)+
					   max(p.getLength()/maxLengthOfEdge,q.getLength()/maxLengthOfEdge)/avgLength);
}

double positionCompatibility(Edge p, Edge q){
	double avgLength = (p.getLength()/maxLengthOfEdge+q.getLength()/maxLengthOfEdge)/2;
	/*double comp = avgLength/(avgLength+(euclidDist(p.getMidPoint(),q.getMidPoint())/maxLengthOfEdge));
	if(comp > 1.0 || comp < 0.0){
		cout << "position compatibility: " << comp << endl;
		system("pause");
	}
	return comp;*/
	return avgLength/(avgLength+(euclidDist(p.getMidPoint(),q.getMidPoint())/maxLengthOfEdge)*(euclidDist(p.getMidPoint(),q.getMidPoint())/maxLengthOfEdge));
}

Point projectPointToEdge(Edge p, Point q){
	double r = ((p.getSource()->getY()-q.getY())*(p.getSource()->getY()-p.getTarget()->getY()) -
				(p.getSource()->getX()-q.getX())*(p.getTarget()->getX()-p.getSource()->getX())) /
				(p.getLength()*p.getLength());
	return Point(p.getSource()->getX()+r*(p.getTarget()->getX()-p.getSource()->getX()),
				 p.getSource()->getY()+r*(p.getTarget()->getY()-p.getSource()->getY()),
				 0.0);
}

double getVisibility(Edge p, Edge q){
	Point I0 = projectPointToEdge(p, Point(q.getSource()->getX(),q.getSource()->getY(),q.getSource()->getZ()));
	Point I1 = projectPointToEdge(p, Point(q.getTarget()->getX(),q.getTarget()->getY(),q.getTarget()->getZ()));
	Point Im = Edge(I0,I1).getMidPoint();
	/*double visibility = max(1-(2*euclidDist(p.getMidPoint(),Im))/(euclidDist(I0,I1)),0);
	if(visibility > 1.0 || visibility < 0.0){
		cout << "visibility compatibility: " << visibility << endl;
		system("pause");
	}
	return visibility;*/
	return max(1-(2*euclidDist(p.getMidPoint(),Im))/(euclidDist(I0,I1)),0);
}

double visibilityCompatibility(Edge p, Edge q){
	/*double comp = min(getVisibility(p,q),getVisibility(q,p));
	if(comp > 1.0 || comp < 0.0){
		cout << "visibility compatibility: " << comp << endl;
		system("pause");
	}
	return 1.0;*/
	return min(getVisibility(p,q),getVisibility(q,p));
}

float getCompatibility(Edge p, Edge q){
	return (float)angleCompatibility(p,q)*scaleCompatibility(p,q)*positionCompatibility(p,q)*visibilityCompatibility(p,q);
}

float getCompatibility(int p, int q){
	return CeArray[p][q];
}

// compatibility measurement for all edges
void getCompatibilityMeasurement(){
	int a = 0;
	int b = 0;
	for(unsigned int i = 0; i < debugCount; i++){
		a = (int)((double)i/(double)debugCount*100.0);
		if(a > b){
			cout << "done: " << a << "%" << endl;
			b = a;
		}
		for(unsigned int j = 0; j < debugCount; j++){
	//for(unsigned int i = 0; i < edges.size()/2; i++){
		//for(unsigned int j = 0; j < edges.size()/2; j++){
			if(i<j){
				CeArray[min(i,j)][max(i,j)] = getCompatibility(edges[i],edges[j]);
			}
		}
	}
}

// edge bundler
void bundle(){
	//for(unsigned int i = 0; i < edges.size(); i++){
		//for(unsigned int j = 0; j < edges.size(); j++){
	for(unsigned int i = 0; i < debugCount; i++){
		for(unsigned int j = 0; j < debugCount; j++){
			if(i!=j){
				double Ce = (double)getCompatibility(min(i,j),max(i,j));
				if(Ce > compatibilityThreshold){
					Vector Fe = Vector(0.0,0.0,0.0);
					double length;
					for(unsigned int k = 1; k < edges[i].getPoints().size()-1; k++){
						Vector v = Vector(edges[i].getPoints()[k], edges[j].getPoints()[k]);
						double m;
						length = v.getLength();
						if(inverseLinearModel){
							m = (Ce / length);
						} else{
							m = (Ce / length) / (length);
						}
						v.multiple(m);
						Fe.add(v);
					}
					for(unsigned int k = 1; k < edges[i].getPoints().size()-1; k++){
						double kp = K/edges[i].getLength();
						Vector v1 = Vector(edges[i].getPoints()[k],edges[i].getPoints()[k-1]);
						Vector v2 = Vector(edges[i].getPoints()[k],edges[i].getPoints()[k+1]);
						v1.add(v2);
						v1.multiple(kp);
						Vector Fs = v1;
						Fs.setZ(0.0);
						Vector Fp = Fs.added(Fe);
						edges[i].setPoint(Point(edges[i].getPoints()[k].getX()+min(Fp.getX()*S,length),
												edges[i].getPoints()[k].getY()+min(Fp.getY()*S,length),
												edges[i].getPoints()[k].getZ()+min(Fp.getZ()*S,length)),k);
					}
				}
			}
		}
	}
}

void restart(){
	for(unsigned int i = 0; i < edges.size(); i++){
		edges[i].restartEdge();
	}
}

void recalculateEdges(){
	for(unsigned int i = 0; i < debugCount; i++){
		edges[i].recalculatePoints();
	}
}

void decreaseEdges(){
	for(unsigned int i = 0; i < debugCount; i++){
		edges[i].decreasePoints();
	}
}

void startBundling(){
	restart();
	for(int cycle = 0; cycle < 5; cycle++){
		recalculateEdges();
		S = Ss[cycle];
		for(int iter = 0; iter < iterations[cycle]; iter++){
			bundle();
		}
	}
}

void drawEarth(){
	glClearColor(0.0,0.0,0.0,1.0);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	GLUquadricObj *quadric;
	quadric = gluNewQuadric();
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gluQuadricTexture(quadric,1);

    gluSphere(quadric,2,15,15);
	gluDeleteQuadric(quadric);
	glRotatef(-90, 1.0f, 0.0f, 0.0f);
}

Airport normalize(Airport a){
	double q = 3.0;
	Airport normalized = Airport(a.getID(), (a.getX()-avgX)/q, (a.getY()-avgY)/q, a.getZ(), a.getName());
	normalized.setDegree(a.getDegree());
	return normalized;
}

Point normalize(Point a){
	double q = 3.0;
	Point normalized = Point((a.getX()-avgX)/q, (a.getY()-avgY)/q, a.getZ());
	return normalized;
}

void setLabel(float x, float y, float z){
	float delta = 0.2f;
	float w = 2.0f;
	float h = 1.0f;
	glBegin(GL_POLYGON);
		glNormal3f(0.0,0.0,1.0);
		glVertex3f(x+delta, y+delta, z);
		glNormal3f(0.0,0.0,1.0);
		glVertex3f(x+delta+w, y+delta, z);
		glNormal3f(0.0,0.0,1.0);
		glVertex3f(x+delta+w, y+delta+h, z);
		glNormal3f(0.0,0.0,1.0);
		glVertex3f(x+delta, y+delta+h, z);
	glEnd();
}

void drawCircle(float x, float y, float z, float r, GLfloat* color){
	float i;
	float s = (3.14 * 2 / 25);
    /*glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, color);
    glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, color);*/
	glBegin(GL_POLYGON);
	for(i = 3.14; i >= -3.14; i -= s)
	{
		glNormal3f(0.0,0.0,1.0);
		glVertex3f(x + sin(i) * r, y + cos(i) * r, z);
	}
	glEnd();
}

void drawSphere(float x, float y, float z, float r, GLfloat* color){
    glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, color);
    glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, color);
	GLUquadricObj *quadric;
	quadric = gluNewQuadric();
    gluQuadricTexture(quadric,1);
	glTranslatef(x,y,z);
    gluSphere(quadric,r,10,10);
	glTranslatef(-x,-y,-z);
	gluDeleteQuadric(quadric);
}

void drawAirports(){
	for(vector<Airport>::iterator it = nodes.begin(); it != nodes.end(); it++){
		if(it->getSelected()){
			Airport normalized = normalize(*it);
			GLfloat color[4] = {normalized.getDegree()/258.0, 1.0-normalized.getDegree()/258.0, 0.0f, 1.0f};
			if(!drawColor){
				color[0] = 0.0;
				color[1] = 0.0;
			}
			//drawCircle(normalized.getX(), normalized.getY(), normalized.getZ(), normalized.getDegree()/150.0 + 0.5, color);
			drawSphere(normalized.getX(), normalized.getY(), normalized.getZ(), normalized.getDegree()/150.0 + 0.5, color);
		} else{
			Airport normalized = normalize(*it);
			GLfloat color[4] = {normalized.getDegree()/258.0, 1.0-normalized.getDegree()/258.0, 0.0f, 1.0f};
			if(!drawColor){
				color[0] = 0.0;
				color[1] = 0.0;
			}
			//drawCircle(normalized.getX(), normalized.getY(), normalized.getZ(), normalized.getDegree()/150.0 + 0.5, color);
			drawSphere(normalized.getX(), normalized.getY(), normalized.getZ(), normalized.getDegree()/150.0 + 0.5, color);
		}
	}
}

void Edge::draw(){
	glLineWidth(edgeWidth);
	if(antialiasing){
		glEnable (GL_LINE_SMOOTH);
	} else{
		glDisable (GL_LINE_SMOOTH);
	}
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float eps = 0.04;
	for(unsigned int i = 0; i < points.size()-1; i++){
		Point normalizedSource = normalize(points[i]);
		Point normalizedTarget = normalize(points[i+1]);
		float t = (float)i/(points.size()-1);
		float t1 = (float)(i+1.0)/(points.size()-1);
		GLfloat colorS[4] = {(1.0-t)*(start->getDegree()/258.0)+t*(end->getDegree()/258.0),
							1.0-((1.0-t)*(start->getDegree()/258.0)+t*(end->getDegree()/258.0)),
							0.0f, 1.0f};
		GLfloat colorT[4] = {(1.0-t1)*(start->getDegree()/258.0)+t1*(end->getDegree()/258.0),
							1.0-((1.0-t1)*(start->getDegree()/258.0)+t1*(end->getDegree()/258.0)),
							0.0f, 1.0f};
		if(!drawColor){
			colorS[0] = 0.0;
			colorS[1] = 0.0;
			colorT[0] = 0.0;
			colorT[1] = 0.0;
		}
		if(drawHeight){
			if(showPoints){
				drawSphere(normalizedSource.getX(), normalizedSource.getY(), normalizedSource.getZ()+0.03,0.3,colorS);
				drawSphere(normalizedTarget.getX(), normalizedTarget.getY(), normalizedTarget.getZ()+0.03,0.3,colorT);
			}
			glBegin(GL_LINES);
				glNormal3f(0.0,0.0,1.0);
				glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, colorS);
				glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, colorS);
				glVertex3f(normalizedSource.getX(), normalizedSource.getY(), normalizedSource.getZ()-eps);
				glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, colorT);
				glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, colorT);
				glVertex3f(normalizedTarget.getX(), normalizedTarget.getY(), normalizedTarget.getZ()-eps);
			glEnd();
		} else{
			if(showPoints){
				drawSphere(normalizedSource.getX(), normalizedSource.getY(), 0.03,0.3,colorS);
				drawSphere(normalizedTarget.getX(), normalizedTarget.getY(), 0.03,0.3,colorT);
			}
			glBegin(GL_LINES);
				glNormal3f(0.0,0.0,1.0);
				glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, colorS);
				glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, colorS);
				glVertex3f(normalizedSource.getX(), normalizedSource.getY(), 0.03);
				glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, colorT);
				glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, colorT);
				glVertex3f(normalizedTarget.getX(), normalizedTarget.getY(), 0.03);
			glEnd();
		}
	}
}

void drawEdges(){
	/*for(unsigned int i = 0; i < edges.size(); i++){
		edges[i].draw();
	}*/
	for(unsigned int i = 0; i < debugCount; i++){
		edges[i].draw();
	}
}

float x = 100;
float y = 70;

void drawPlane(){
	glClearColor(1.0,1.0,1.0,1.0);

	glRotatef(-scene_rot, 0.0f, 1.0f, 0.0f);
	float rotZ = 0.0f;
	float dx=-6.0f;
	float dy=4.0f;
	float dz=0.0f;
	float cropX = 1.0f;
	glRotatef(rotZ, 0.0f, 0.0f, 1.0f);
	glTranslatef(dx,dy,dz);
	if(!hide){
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textureID1);
		glBegin(GL_QUADS);
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f(0.0,0.0);
			glVertex3f(-x,-y,0.0);
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f(1.0,0.0);
			glVertex3f(x*cropX,-y,0.0);
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f(1.0,1.0);
			glVertex3f(x/cropX,y,0.0);
			glNormal3f(0.0,0.0,1.0);
			glTexCoord2f(0.0,1.0);
			glVertex3f(-x,y,0.0);
		glEnd();
	}
	glTranslatef(-dx,-dy,-dz);
	glRotatef(-rotZ, 0.0f, 0.0f, 1.0f);


	// airport points
	glDisable(GL_TEXTURE);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	GLfloat color[4] = {0.9f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, color);
    glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, color);
	drawAirports();

	color[0] = 0.1f;
	color[1] = 0.1f;
	color[2] = 0.1f;
    glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, color);
    glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, color);
	if(drawEdgesFlag){
		drawEdges();
	}

	// default color
	GLfloat material_amd[4] = {0.9f, 0.9f, 0.9f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, material_amd);
    glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, material_amd);

	// default rotation
	glRotatef(scene_rot, 0.0f, 1.0f, 0.0f);
}

//-----------------------------------------------------------------------------
// Name: cbDisplay()
// Desc: 
//-----------------------------------------------------------------------------
void cbDisplay()
{
    //static GLuint s_tex_id = prg2CreateSimpleTexture(256, 256);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, g_WireMode ? GL_LINE : GL_FILL);
    (g_FaceCulling) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);

    // Setup camera
    glLoadIdentity();
    glTranslatef(-g_SceneTraX, -g_SceneTraY, -g_SceneTraZ);
    pgr2AddQuaternionRotationToTransformation(g_SceneRot);
	if (g_GeometryType == SPHERE_GEOMETRY)
		pgr2AddQuaternionRotationToTransformation(g_SceneRot1);
	
    glRotatef(scene_rot, 0.0f, 1.0f, 0.0f);

    // Turn on programmable pipeline
    if (g_UseShaders)
    {
        glUseProgram(g_ProgramId);    // Active shader program

		GLint hTimer = glGetUniformLocation(g_ProgramId, "u_Timer");
		assert(hTimer > -1);
		static int s_counter = 0;
		glUniform1f(hTimer, (++s_counter % 200) / 200.0);
    }

    // Draw model
    switch (g_GeometryType)
    {
		case SPHERE_GEOMETRY   :
			glEnable(GL_TEXTURE_2D);
			drawEarth();
			break;
		case PLANE_GEOMETRY:
			glEnable(GL_TEXTURE_2D);
			drawPlane();
			break;
    }

    // Turn off programmable pipeline
    glUseProgram(NULL);

    if (g_SceneRotEnabled)
    {
		scene_rot += g_RotSpeed;
    }
}

//-----------------------------------------------------------------------------
// Name: cbInitGL()
// Desc: 
//-----------------------------------------------------------------------------
void cbInitGL()
{
    // Init app GUI
    initGUI();

	LoadGLTextures();

    // Set OpenGL state variables
    glClearColor(0.0f, 0.0f, 0.0f, 0);
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    (g_FaceCulling) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);

    GLfloat light_amb[4] = {0.4f, 0.4f, 0.4f, 1.0f};
    GLfloat light_dif[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat light_spe[4] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat light_pos[4] = {0.0f, 5.0f, 5.0f, 1.0f};
    GLfloat light_dir[4] = {0.0f,-1.5f,-3.0f, 1.0f};
	
    GLfloat material_amd[4] = {0.9f, 0.9f, 0.9f, 1.0f};
    //GLfloat material_spe[4] = {0.8f, 0.8f, 0.8f, 1.0f};

    glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, material_amd);
    glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, material_amd);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_spe);
    //glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION      , light_pos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light_dir);
    glLightfv(GL_LIGHT1, GL_AMBIENT       , light_amb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE       , light_dif);
    //glLightfv(GL_LIGHT1, GL_SPECULAR	  , light_spe);
    //glLightf (GL_LIGHT1, GL_SPOT_CUTOFF	 , 24);
    //glLightf (GL_LIGHT1, GL_SPOT_EXPONENT , 128);
	
    cbCompileShaderProgram(NULL);
}


//-----------------------------------------------------------------------------
// Name: cbCompileShaderProgram()
// Desc: 
//-----------------------------------------------------------------------------
void TW_CALL cbCompileShaderProgram(void *clientData)
{
    // Delete shader program if exists
    if (g_ProgramId)
    {
        glDeleteProgram(g_ProgramId);
    }

    // Create shader program object
    g_ProgramId = glCreateProgram();

    if (g_UseVertexShader)
    {
        // Create shader objects for vertex shader
        GLuint id = pgr2CreateShaderFromFile(GL_VERTEX_SHADER, VS_FILE_NAME);
        glAttachShader(g_ProgramId, id);
        glDeleteShader(id);
    }
    if (g_UseGeometryShader)
    {
        // Create shader objects for geometry shader
        GLuint id = pgr2CreateShaderFromFile(GL_GEOMETRY_SHADER, GS_FILE_NAME);
        glAttachShader(g_ProgramId, id);
        glDeleteShader(id);
        glProgramParameteriEXT(g_ProgramId, GL_GEOMETRY_VERTICES_OUT_EXT, 6);
        glProgramParameteriEXT(g_ProgramId, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
        glProgramParameteriEXT(g_ProgramId, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
    }
    if (g_UseFragmentShader)
    {
        // Create shader objects for fragment shader
        GLuint id = pgr2CreateShaderFromFile(GL_FRAGMENT_SHADER, FS_FILE_NAME);
        glAttachShader(g_ProgramId, id);
        glDeleteShader(id);
    }

    // Link shader program
    glLinkProgram(g_ProgramId);
    if (!pgr2CheckProgramLinkStatus(g_ProgramId))
    {
        pgr2CheckProgramInfoLog(g_ProgramId);
        printf("Shader program creation failed.\n\n");
        glDeleteProgram(g_ProgramId);
        g_ProgramId  = 0;
        g_UseShaders = false;
        return;
    }
    else
    {
        printf("Shader program compiled successfully.\n\n");
    }
}

//-----------------------------------------------------------------------------
// Name: initGUI()
// Desc: 
//-----------------------------------------------------------------------------
void initGUI()
{
#ifdef USE_ANTTWEAKBAR
    // Initialize AntTweakBar GUI
    if (!TwInit(TW_OPENGL, NULL))
    {
        assert(0);
    }

    TwWindowSize(g_WindowWidth, g_WindowHeight);
    TwBar *controlBar = TwNewBar("Controls");
    TwDefine(" Controls position='10 10' size='200 360' refresh=0.1 ");

    //TwAddVarCB(controlBar, "use_shaders", TW_TYPE_BOOLCPP, cbSetShaderStatus, cbGetShaderStatus, NULL, " label='shaders' key=s help='Turn programmable pipeline on/off.' ");

    // Shader panel setup
    //TwAddVarRW(controlBar, "vs", TW_TYPE_BOOLCPP, &g_UseVertexShader, " group='Shaders' label='vertex' key=v help='Toggle vertex shader.' ");
    //TwAddVarRW(controlBar, "gs", TW_TYPE_BOOLCPP, &g_UseGeometryShader, " group='Shaders' label='geometry' key=g help='Toggle geometry shader.' ");
    //TwAddVarRW(controlBar, "fs", TW_TYPE_BOOLCPP, &g_UseFragmentShader, " group='Shaders' label='fragment' key=f help='Toggle fragment shader.' ");
    //TwAddButton(controlBar, "build", cbCompileShaderProgram, NULL, " group='Shaders' label='build' key=b help='Build shader program.' ");
//  TwDefine( " Controls/Shaders readonly=true "); 

    // Render panel setup
    TwAddVarRW(controlBar, "wiremode", TW_TYPE_BOOLCPP, &g_WireMode, " group='Render' label='wire mode' key=w help='Toggle wire mode.' ");
    TwAddVarRW(controlBar, "face_culling", TW_TYPE_BOOLCPP, &g_FaceCulling, " group=Render label='face culling' key=c help='Toggle face culling.' ");
	TwAddVarRW(controlBar, "Constant K", TW_TYPE_DOUBLE, &K, " group='Scene' label='K constant' min=0.1 max=1000 step=0.1 keyIncr=K keyDecr=k help='K constant' ");
	TwAddVarRW(controlBar, "Constant S", TW_TYPE_DOUBLE, &S, " group='Scene' label='S constant' min=0.0 max=1000 step=0.001 keyIncr=Z keyDecr=z help='S constant' ");

    // Scene panel setup
    TwEnumVal geometry_type[] = 
    {
		{ SPHERE_GEOMETRY	   , "Sphere"  },
		{ PLANE_GEOMETRY	   , "Plane"  },
    };
    TwType geom_type = TwDefineEnum("Model", geometry_type, NUM_GEOMETRY_TYPES);
    //TwAddVarRW(controlBar, "model", geom_type, &g_GeometryType, " group='Scene' keyIncr=Space help='Change model.' ");
    TwAddVarRW(controlBar, "auto-rotation", TW_TYPE_BOOLCPP, &g_SceneRotEnabled, " group='Scene' label='rotation' key=r help='Toggle scene rotation.' ");
	TwAddVarRW(controlBar, "Edge width", TW_TYPE_FLOAT, &edgeWidth, " group='Scene' label='edge width' min=0.1 max=3 step=0.1 keyIncr=3 keyDecr=1 help='Edge width.' ");
	TwAddVarRW(controlBar, "Number of edges", TW_TYPE_INT16, &debugCount, " group='Scene' label='edges number' min=0 max=2101 step=1 keyIncr=9 keyDecr=7 help='Edges number.' ");
	TwAddVarRW(controlBar, "Threshold", TW_TYPE_DOUBLE, &compatibilityThreshold, " group='Scene' label='threshold' min=0.0 max=1.0 step=0.1 keyIncr=T keyDecr=t help='compatibility threshold.' ");
    TwAddVarRW(controlBar, "Translate", TW_TYPE_FLOAT, &g_SceneTraZ, " group='Scene' label='translate' min=1 max=1000 step=1.0 keyIncr=- keyDecr=+ help='Scene translation.' ");
	TwAddVarRW(controlBar, "Translate X", TW_TYPE_FLOAT, &g_SceneTraX, " group='Scene' label='translate' min=-100 max=100 step=1.0 keyIncr=6 keyDecr=4 help='Scene translation.' ");
	TwAddVarRW(controlBar, "Translate Y", TW_TYPE_FLOAT, &g_SceneTraY, " group='Scene' label='translate' min=-100 max=100 step=1.0 keyIncr=8 keyDecr=2 help='Scene translation.' ");
    TwAddVarRW(controlBar, "Rotation-speed", TW_TYPE_FLOAT, &g_RotSpeed, " group='Scene' label='speed' min=0.1 max=10 step=0.1 keyIncr=m keyDecr=M help='Rotation speed.' ");
	TwAddVarRW(controlBar, "SceneRotation", TW_TYPE_QUAT4F, &g_SceneRot, " group='Scene' label='rotation' open help='Toggle scene orientation.' ");
#endif
}


//-----------------------------------------------------------------------------
// Name: cbWindowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void cbWindowSizeChanged(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(55.0f, GLfloat(width)/height, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);

    g_WindowWidth  = width;
    g_WindowHeight = height;
}


//-----------------------------------------------------------------------------
// Name: cbKeyboardChanged()
// Desc: 
//-----------------------------------------------------------------------------
void cbKeyboardChanged(int key, int action)
{
    switch (key)
    {
    case GLFW_KEY_SPACE:
        g_GeometryType = (g_GeometryType + 1) % NUM_GEOMETRY_TYPES;
		g_SceneTraZ = (g_GeometryType == 0)?5.0f:100.0f;
        break;
    //case 'v' : g_UseVertexShader   = !g_UseVertexShader;                 break;
    //case 'g' : g_UseGeometryShader = !g_UseGeometryShader;               break;
    //case 'f' : g_UseFragmentShader = !g_UseFragmentShader;               break;
    case 'w' : g_WireMode          = !g_WireMode;                        break;
    case 'c' : g_FaceCulling       = !g_FaceCulling;                     break;
	case 'C' : drawColor		   = !drawColor;			             break;
    //case 's' : g_UseShaders			= !g_UseShaders;                     break;
	case 'h' : hide					= !hide;                             break;
	case 'H' : drawHeight			= !drawHeight;					     break;
	case 'a' : antialiasing			= !antialiasing;                     break;
	case 'd' : decreaseEdges();							                 break;
	case 'q' : recalculateEdges();						                 break;
	case 'Q' : bundle();								                 break;
	case 'R' : restart();								                 break;
	case 'e' : drawEdgesFlag = !drawEdgesFlag;			                 break;
	case 'i' : inverseLinearModel = !inverseLinearModel;                 break;
	case 'p' : showPoints = !showPoints;								 break;
	case 'B' : startBundling();											 break;
    /*case 'b' : 
        cbCompileShaderProgram(NULL);
        return;
        break;*/
    }

    /*printf("[t/T] g_SceneTraZ         = %f\n", g_SceneTraZ);
    printf("[r]   g_SceneRotEnabled   = %s\n", g_SceneRotEnabled ? "true" : "false");
    printf("[w]   g_WireMode          = %s\n", g_WireMode ? "true" : "false");
    printf("[c]   g_FaceCulling       = %s\n\n", g_FaceCulling ? "true" : "false");
    //printf("[s]   g_UseShaders        = %s\n", g_UseShaders ? "true" : "false");
    //printf("[v]   g_UseVertexShader   = %s\n", g_UseVertexShader ? "true" : "false");
    //printf("[g]   g_UseGeometryShader = %s\n", g_UseGeometryShader ? "true" : "false");
    //printf("[f]   g_UseFragmentShader = %s\n", g_UseFragmentShader ? "true" : "false");
    //printf("[b]   re-compile shaders\n\n");*/
}


#ifdef USE_ANTTWEAKBAR
//-----------------------------------------------------------------------------
// Name: cbSetShaderStatus()
// Desc: 
//-----------------------------------------------------------------------------
void TW_CALL cbSetShaderStatus(const void *value, void *clientData)
{
    g_UseShaders = *(bool*)(value);
    // Try to compile shader program
    if (g_UseShaders)
    {
        cbCompileShaderProgram(NULL);
    }
//  TwDefine((g_UseShaders) ? " Controls/Shaders readonly=false " : " Controls/Shaders readonly=true "); 
}


//-----------------------------------------------------------------------------
// Name: cbGetShaderStatus()
// Desc: 
//-----------------------------------------------------------------------------
void TW_CALL cbGetShaderStatus(void *value, void *clientData)
{
    *(bool*)(value) = g_UseShaders;
} 
#else
bool g_MouseRotationEnabled = false;

//-----------------------------------------------------------------------------
// Name: cbMouseButtonChanged()
// Desc: internal
//-----------------------------------------------------------------------------
void GLFWCALL cbMouseButtonChanged(int button, int action)
{
    g_MouseRotationEnabled = ((button == GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS));
}


//-----------------------------------------------------------------------------
// Name: cbMousePositionChanged()
// Desc: 
//-----------------------------------------------------------------------------
void cbMousePositionChanged(int x, int y)
{
    static int s_LastMousePoxX = x;
    static int s_LastMousePoxY = y;

    if (g_MouseRotationEnabled)
    {
        g_SceneRot[1] +=  0.9f*(x - s_LastMousePoxX);
        g_SceneRot[2] +=  0.9f*(y - s_LastMousePoxY);
        s_LastMousePoxX = x;
        s_LastMousePoxY = y;
    }
}
#endif

void parseXML(char* fileName){
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(fileName);
	cout << "XML load result: " << result.description() << std::endl;
	pugi::xml_node graphml = doc.child("graphml");
	pugi::xml_node graph = graphml.child("graph");
	for(pugi::xml_node_iterator it = graph.begin(); it != graph.end(); it++) {
		if(!strcmp(it->name(), "node")){
			double x = 0.0;
			double y = 0.0;
			double z = 0.05;
			string name = "";
			for(pugi::xml_node_iterator it1 = it->begin(); it1 != it->end(); it1++) {
				if(!strcmp(it1->attribute("key").as_string(), "x")){
					x = it1->text().as_double();
				} else if(!strcmp(it1->attribute("key").as_string(), "y")){
					y = it1->text().as_double();
				} else if(!strcmp(it1->attribute("key").as_string(), "tooltip")){
					string n = it1->text().as_string();
					name = n.substr(0,3);
				}
			}
			nodes.push_back(Airport(it->attribute("id").as_int(), x, y, z, name));
		} else if(!strcmp(it->name(), "edge")){
			//edges.push_back(Edge(it->attribute("id").as_int(),it->attribute("source").as_int(), it->attribute("target").as_int()));
			edges.push_back(Edge(it->attribute("id").as_int(), &nodes[it->attribute("source").as_int()], &nodes[it->attribute("target").as_int()]));
		}
	}
	
	//cout << "node 1: " << nodes[3].getID() << ", " << nodes[3].getName() << ", " << nodes[3].getX() << ", " << nodes[3].getY() << ", " << nodes[3].getZ() << endl;

	cout << "XML parsing done" << endl;
}

void getAveragePoint(){
	double x1 = 0.0;
	double y1 = 0.0;
	for(vector<Airport>::iterator it = nodes.begin(); it != nodes.end(); it++){
		x1 += it->getX();
		y1 += it->getY();
	}
	avgX = x1/nodes.size();
	avgY = y1/nodes.size();
}

void findMaxLength(){
	double max = 0.0;
	for(vector<Edge>::iterator it = edges.begin(); it != edges.end(); it++){
		if(max < it->getLength())
			max = it->getLength();
	}
	maxLengthOfEdge = max;
}

void sort(){
	vector<Edge> temp;
	for(unsigned int i = 0; i < debugCount; i++){
		double max = 0.0;
		int id = 0;
		for(unsigned int j = 0; j < debugCount; j++){
			if(max < edges[j].getLength()){
				max = edges[j].getLength();
				id = j;
			}
		}
		temp.push_back(edges[id]);
		edges[id] = Edge();
	}
	edges.clear();
	for(unsigned int i = 0; i < debugCount; i++){
		edges.push_back(temp[i]);
	}
}

void processGraph(){
	for(vector<Edge>::iterator it = edges.begin(); it != edges.end(); it++){
		nodes[it->getSourceID()].increaseDegree();
		nodes[it->getTargetID()].increaseDegree();
	}

	int max = 0;
	int air = 0;
	for(vector<Airport>::iterator it = nodes.begin(); it != nodes.end(); it++){
		int deg = it->getDegree();
		if(deg > max){
			max = deg;
			air = it->getID();
		}
	}
	sort();
	//cout << "max: " << max << ", id: " << air << endl;
	cout << "number of nodes: " << nodes.size() << endl;
	cout << "number of edges: " << edges.size() << endl;
}

void bundleInit(){
	getAveragePoint();
	processGraph();
	findMaxLength();
	getCompatibilityMeasurement();
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) 
{
	parseXML(argv[1]);
	bundleInit();
	
    return common_main(g_WindowWidth, g_WindowHeight,
                       "[VIZ] US airports network",
                       cbInitGL,              // init GL callback function
                       cbDisplay,             // display callback function
                       cbWindowSizeChanged,   // window resize callback function
                       cbKeyboardChanged,     // keyboard callback function
#ifdef USE_ANTTWEAKBAR
                       NULL,                  // mouse button callback function
                       NULL                   // mouse motion callback function
#else
                       cbMouseButtonChanged,  // mouse button callback function
                       cbMousePositionChanged // mouse motion callback function
#endif
                       );
}
