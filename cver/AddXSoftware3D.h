#ifndef ADDX_SOFTWARE_3D_H
#define ADDX_SOFTWARE_3D_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_VERTICES 10000
#define MAX_TRIANGLES 5000
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    Vec3 vertices[3];
    float r, g, b, a;
} Triangle;

typedef struct {
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
} Transform;

typedef struct {
    float m[16];
} Matrix4x4;

typedef struct {
    uint32_t pixels[SCREEN_HEIGHT][SCREEN_WIDTH];
    uint32_t zbuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
    HWND windowHandle;
    HDC memoryDC;
    HBITMAP bitmap;
    uint32_t width;
    uint32_t height;
    Triangle triangles[MAX_TRIANGLES];
    int triangleCount;
    Matrix4x4 projectionMatrix;
    Matrix4x4 viewMatrix;
    Matrix4x4 modelMatrix;
    bool clearColor;
    float bgR, bgG, bgB, bgA;
} SoftwareRenderer;

Matrix4x4 Matrix4x4_Identity(void);
Matrix4x4 Matrix4x4_Multiply(Matrix4x4 a, Matrix4x4 b);
Matrix4x4 Matrix4x4_Translate(float x, float y, float z);
Matrix4x4 Matrix4x4_RotateX(float angle);
Matrix4x4 Matrix4x4_RotateY(float angle);
Matrix4x4 Matrix4x4_RotateZ(float angle);
Matrix4x4 Matrix4x4_Scale(float x, float y, float z);
Matrix4x4 Matrix4x4_Perspective(float fov, float aspect, float near, float far);
Matrix4x4 Matrix4x4_LookAt(Vec3 eye, Vec3 target, Vec3 up);

Vec3 Vec3_Add(Vec3 a, Vec3 b);
Vec3 Vec3_Sub(Vec3 a, Vec3 b);
Vec3 Vec3_Mul(Vec3 a, float s);
Vec3 Vec3_Cross(Vec3 a, Vec3 b);
float Vec3_Dot(Vec3 a, Vec3 b);
float Vec3_Length(Vec3 v);
Vec3 Vec3_Normalize(Vec3 v);

void SoftwareRenderer_Init(SoftwareRenderer* r);
void SoftwareRenderer_Destroy(SoftwareRenderer* r);
bool SoftwareRenderer_Create(SoftwareRenderer* r, uint32_t width, uint32_t height, const char* title);

void SoftwareRenderer_Clear(SoftwareRenderer* r, float r5, float g, float b, float a);
void SoftwareRenderer_ClearDepth(SoftwareRenderer* r);

void SoftwareRenderer_SetProjection(SoftwareRenderer* r, Matrix4x4 m);
void SoftwareRenderer_SetView(SoftwareRenderer* r, Matrix4x4 m);
void SoftwareRenderer_SetModel(SoftwareRenderer* r, Matrix4x4 m);

void SoftwareRenderer_AddTriangle(SoftwareRenderer* r, Vec3 v1, Vec3 v2, Vec3 v3, float r2, float g, float b, float a);
void SoftwareRenderer_AddCube(SoftwareRenderer* r, Vec3 pos, float size, float r1, float g, float b, float a);
void SoftwareRenderer_AddSphere(SoftwareRenderer* r, Vec3 pos, float radius, int segments, float r3, float g, float b, float a);
void SoftwareRenderer_AddPlane(SoftwareRenderer* r, Vec3 pos, Vec3 normal, float r4, float g, float b, float a);

void SoftwareRenderer_Render(SoftwareRenderer* r);
void SoftwareRenderer_Present(SoftwareRenderer* r);

#endif