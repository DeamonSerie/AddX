#include "AddXSoftware3D.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Matrix4x4 Matrix4x4_Identity(void) {
    Matrix4x4 m = {{0}};
    m.m[0] = m.m[5] = m.m[10] = m.m[15] = 1.0f;
    return m;
}

Matrix4x4 Matrix4x4_Multiply(Matrix4x4 a, Matrix4x4 b) {
    Matrix4x4 r = {{0}};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                r.m[i * 4 + j] += a.m[i * 4 + k] * b.m[k * 4 + j];
            }
        }
    }
    return r;
}

Matrix4x4 Matrix4x4_Translate(float x, float y, float z) {
    Matrix4x4 m = Matrix4x4_Identity();
    m.m[3] = x; m.m[7] = y; m.m[11] = z;
    return m;
}

Matrix4x4 Matrix4x4_RotateX(float angle) {
    float c = cosf(angle), s = sinf(angle);
    Matrix4x4 m = Matrix4x4_Identity();
    m.m[5] = c; m.m[6] = -s;
    m.m[9] = s; m.m[10] = c;
    return m;
}

Matrix4x4 Matrix4x4_RotateY(float angle) {
    float c = cosf(angle), s = sinf(angle);
    Matrix4x4 m = Matrix4x4_Identity();
    m.m[0] = c; m.m[2] = s;
    m.m[8] = -s; m.m[10] = c;
    return m;
}

Matrix4x4 Matrix4x4_RotateZ(float angle) {
    float c = cosf(angle), s = sinf(angle);
    Matrix4x4 m = Matrix4x4_Identity();
    m.m[0] = c; m.m[1] = -s;
    m.m[4] = s; m.m[5] = c;
    return m;
}

Matrix4x4 Matrix4x4_Scale(float x, float y, float z) {
    Matrix4x4 m = {{0}};
    m.m[0] = x; m.m[5] = y; m.m[10] = z; m.m[15] = 1.0f;
    return m;
}

Matrix4x4 Matrix4x4_Perspective(float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov / 2.0f);
    Matrix4x4 m = {{0}};
    m.m[0] = f / aspect;
    m.m[5] = f;
    m.m[10] = (fov + aspect) / (aspect - fov);
    m.m[11] = (2.0f * fov * aspect) / (aspect - fov);
    m.m[14] = -1.0f;
    return m;
}

Matrix4x4 Matrix4x4_LookAt(Vec3 eye, Vec3 target, Vec3 up) {
    Vec3 z = Vec3_Normalize(Vec3_Sub(eye, target));
    Vec3 x = Vec3_Normalize(Vec3_Cross(up, z));
    Vec3 y = Vec3_Cross(z, x);
    
    Matrix4x4 m = Matrix4x4_Identity();
    m.m[0] = x.x; m.m[4] = x.y; m.m[8] = x.z;
    m.m[1] = y.x; m.m[5] = y.y; m.m[9] = y.z;
    m.m[2] = z.x; m.m[6] = z.y; m.m[10] = z.z;
    m.m[3] = -Vec3_Dot(x, eye);
    m.m[7] = -Vec3_Dot(y, eye);
    m.m[11] = -Vec3_Dot(z, eye);
    return m;
}

Vec3 Vec3_Add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 Vec3_Sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 Vec3_Mul(Vec3 a, float s) {
    return (Vec3){a.x * s, a.y * s, a.z * s};
}

Vec3 Vec3_Cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float Vec3_Dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Vec3_Length(Vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 Vec3_Normalize(Vec3 v) {
    float len = Vec3_Length(v);
    if (len > 0.0001f) {
        return (Vec3){v.x / len, v.y / len, v.z / len};
    }
    return (Vec3){0, 0, 0};
}

static Vec3 transformVec3(Vec3 v, Matrix4x4 m) {
    float w = v.x * m.m[3] + v.y * m.m[7] + v.z * m.m[11] + m.m[15];
    return (Vec3){
        (v.x * m.m[0] + v.y * m.m[4] + v.z * m.m[8] + m.m[12]) / w,
        (v.x * m.m[1] + v.y * m.m[5] + v.z * m.m[9] + m.m[13]) / w,
        (v.x * m.m[2] + v.y * m.m[6] + v.z * m.m[10] + m.m[14]) / w
    };
}

static Vec3 projectVec3(Vec3 v, int w, int h) {
    return (Vec3){
        (v.x + 1.0f) * 0.5f * w,
        (1.0f - v.y) * 0.5f * h,
        v.z
    };
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    return DefWindowProcA(hwnd, msg, w, l);
}

void SoftwareRenderer_Init(SoftwareRenderer* r) {
    memset(r, 0, sizeof(SoftwareRenderer));
    r->width = SCREEN_WIDTH;
    r->height = SCREEN_HEIGHT;
    r->bgR = 0.0f; r->bgG = 0.0f; r->bgB = 0.0f; r->bgA = 1.0f;
    r->projectionMatrix = Matrix4x4_Perspective(1.0472f, 4.0f/3.0f, 0.1f, 100.0f);
    r->viewMatrix = Matrix4x4_Identity();
    r->modelMatrix = Matrix4x4_Identity();
}

void SoftwareRenderer_Destroy(SoftwareRenderer* r) {
    if (r->bitmap) DeleteObject(r->bitmap);
    if (r->memoryDC) DeleteDC(r->memoryDC);
    if (r->windowHandle) DestroyWindow(r->windowHandle);
}

bool SoftwareRenderer_Create(SoftwareRenderer* r, uint32_t width, uint32_t height, const char* title) {
    r->width = width;
    r->height = height;
    
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = wndProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "AddX3D";
    RegisterClassA(&wc);
    
    r->windowHandle = CreateWindowA("AddX3D", title ? title : "AddX 3D",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, wc.hInstance, NULL);
    if (!r->windowHandle) return false;
    
    HDC dc = GetDC(r->windowHandle);
    r->memoryDC = CreateCompatibleDC(dc);
    r->bitmap = CreateCompatibleBitmap(dc, width, height);
    SelectObject(r->memoryDC, r->bitmap);
    ReleaseDC(r->windowHandle, dc);
    
    ShowWindow(r->windowHandle, SW_SHOW);
    return true;
}

void SoftwareRenderer_Clear(SoftwareRenderer* r, float cr, float cg, float cb, float ca) {
    r->bgR = cr; r->bgG = cg; r->bgB = cb; r->bgA = ca;
    for (int y = 0; y < r->height; y++) {
        for (int x = 0; x < r->width; x++) {
            r->pixels[y][x] = ((int)(cr * 255) << 16) | ((int)(cg * 255) << 8) | (int)(cb * 255);
            r->zbuffer[y][x] = 0xFFFFFFFF;
        }
    }
}

void SoftwareRenderer_ClearDepth(SoftwareRenderer* r) {
    for (int y = 0; y < r->height; y++) {
        for (int x = 0; x < r->width; x++) {
            r->zbuffer[y][x] = 0xFFFFFFFF;
        }
    }
}

void SoftwareRenderer_SetProjection(SoftwareRenderer* r, Matrix4x4 m) {
    r->projectionMatrix = m;
}

void SoftwareRenderer_SetView(SoftwareRenderer* r, Matrix4x4 m) {
    r->viewMatrix = m;
}

void SoftwareRenderer_SetModel(SoftwareRenderer* r, Matrix4x4 m) {
    r->modelMatrix = m;
}

void SoftwareRenderer_AddTriangle(SoftwareRenderer* r, Vec3 v1, Vec3 v2, Vec3 v3, float cr, float cg, float cb, float ca) {
    if (r->triangleCount >= MAX_TRIANGLES) return;
    
    Triangle* t = &r->triangles[r->triangleCount];
    t->vertices[0] = v1;
    t->vertices[1] = v2;
    t->vertices[2] = v3;
    t->r = cr; t->g = cg; t->b = cb; t->a = ca;
    r->triangleCount++;
}

void SoftwareRenderer_AddCube(SoftwareRenderer* r, Vec3 pos, float size, float cr, float cg, float cb, float ca) {
    float h = size / 2.0f;
    
    Vec3 v[8] = {
        {pos.x - h, pos.y - h, pos.z - h},
        {pos.x + h, pos.y - h, pos.z - h},
        {pos.x + h, pos.y + h, pos.z - h},
        {pos.x - h, pos.y + h, pos.z - h},
        {pos.x - h, pos.y - h, pos.z + h},
        {pos.x + h, pos.y - h, pos.z + h},
        {pos.x + h, pos.y + h, pos.z + h},
        {pos.x - h, pos.y + h, pos.z + h}
    };
    
    int faces[6][4] = {
        {0, 1, 2, 3}, {4, 5, 6, 7}, {0, 1, 5, 4},
        {2, 3, 7, 6}, {0, 3, 7, 4}, {1, 2, 6, 5}
    };
    
    for (int f = 0; f < 6; f++) {
        SoftwareRenderer_AddTriangle(r, v[faces[f][0]], v[faces[f][1]], v[faces[f][2]], cr, cg, cb, ca);
        SoftwareRenderer_AddTriangle(r, v[faces[f][0]], v[faces[f][2]], v[faces[f][3]], cr, cg, cb, ca);
    }
}

void SoftwareRenderer_AddSphere(SoftwareRenderer* r, Vec3 pos, float radius, int segments, float cr, float cg, float cb, float ca) {
    for (int lat = 0; lat < segments; lat++) {
        float theta1 = (float)lat / segments * 3.14159f;
        float theta2 = (float)(lat + 1) / segments * 3.14159f;
        
        for (int lon = 0; lon < segments * 2; lon++) {
            float phi1 = (float)lon / (segments * 2) * 2.0f * 3.14159f;
            float phi2 = (float)(lon + 1) / (segments * 2) * 2.0f * 3.14159f;
            
            Vec3 p1 = {
                pos.x + radius * sinf(theta1) * cosf(phi1),
                pos.y + radius * cosf(theta1),
                pos.z + radius * sinf(theta1) * sinf(phi1)
            };
            Vec3 p2 = {
                pos.x + radius * sinf(theta2) * cosf(phi1),
                pos.y + radius * cosf(theta2),
                pos.z + radius * sinf(theta2) * sinf(phi1)
            };
            Vec3 p3 = {
                pos.x + radius * sinf(theta2) * cosf(phi2),
                pos.y + radius * cosf(theta2),
                pos.z + radius * sinf(theta2) * sinf(phi2)
            };
            Vec3 p4 = {
                pos.x + radius * sinf(theta1) * cosf(phi2),
                pos.y + radius * cosf(theta1),
                pos.z + radius * sinf(theta1) * sinf(phi2)
            };
            
            SoftwareRenderer_AddTriangle(r, p1, p2, p4, cr, cg, cb, ca);
            SoftwareRenderer_AddTriangle(r, p2, p3, p4, cr, cg, cb, ca);
        }
    }
}

void SoftwareRenderer_AddPlane(SoftwareRenderer* r, Vec3 pos, Vec3 normal, float cr, float cg, float cb, float ca) {
    Vec3 tangent = (fabsf(normal.y) < 0.9f) ? 
        Vec3_Normalize(Vec3_Cross(normal, (Vec3){0, 1, 0})) :
        Vec3_Normalize(Vec3_Cross(normal, (Vec3){1, 0, 0}));
    Vec3 bitangent = Vec3_Cross(normal, tangent);
    
    Vec3 corners[4] = {
        Vec3_Add(Vec3_Add(pos, Vec3_Mul(tangent, -1.0f)), Vec3_Mul(bitangent, -1.0f)),
        Vec3_Add(Vec3_Add(pos, Vec3_Mul(tangent, 1.0f)), Vec3_Mul(bitangent, -1.0f)),
        Vec3_Add(Vec3_Add(pos, Vec3_Mul(tangent, 1.0f)), Vec3_Mul(bitangent, 1.0f)),
        Vec3_Add(Vec3_Add(pos, Vec3_Mul(tangent, -1.0f)), Vec3_Mul(bitangent, 1.0f))
    };
    
    SoftwareRenderer_AddTriangle(r, corners[0], corners[1], corners[2], cr, cg, cb, ca);
    SoftwareRenderer_AddTriangle(r, corners[0], corners[2], corners[3], cr, cg, cb, ca);
}

void SoftwareRenderer_Render(SoftwareRenderer* r) {
    Matrix4x4 mvp = Matrix4x4_Multiply(Matrix4x4_Multiply(r->projectionMatrix, r->viewMatrix), r->modelMatrix);
    
    for (int i = 0; i < r->triangleCount; i++) {
        Triangle* tri = &r->triangles[i];
        
        Vec3 v0 = transformVec3(tri->vertices[0], mvp);
        Vec3 v1 = transformVec3(tri->vertices[1], mvp);
        Vec3 v2 = transformVec3(tri->vertices[2], mvp);
        
        Vec3 edge1 = Vec3_Sub(v1, v0);
        Vec3 edge2 = Vec3_Sub(v2, v0);
        float faceNormal = Vec3_Dot(edge1, edge2);
        
        if (faceNormal > 0) continue;
        
        Vec3 p0 = projectVec3(v0, r->width, r->height);
        Vec3 p1 = projectVec3(v1, r->width, r->height);
        Vec3 p2 = projectVec3(v2, r->width, r->height);
        
        int minX = (int)fminf(fminf(p0.x, p1.x), p2.x);
        int maxX = (int)fmaxf(fmaxf(p0.x, p1.x), p2.x);
        int minY = (int)fminf(fminf(p0.y, p1.y), p2.y);
        int maxY = (int)fmaxf(fmaxf(p0.y, p1.y), p2.y);
        
        minX = max(0, minX);
        maxX = min(r->width - 1, maxX);
        minY = max(0, minY);
        maxY = min(r->height - 1, maxY);
        
        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                float w0 = ((p1.y - p2.y) * (x - p2.x) + (p2.x - p1.x) * (y - p2.y)) / ((p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y));
                float w1 = ((p2.y - p0.y) * (x - p2.x) + (p0.x - p2.x) * (y - p2.y)) / ((p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y));
                float w2 = 1.0f - w0 - w1;
                
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    float z = w0 * p0.z + w1 * p1.z + w2 * p2.z;
                    
                    if (z < r->zbuffer[y][x]) {
                        r->zbuffer[y][x] = (uint32_t)(z * 4294967295.0f);
                        r->pixels[y][x] = ((int)(tri->r * 255) << 16) | ((int)(tri->g * 255) << 8) | (int)(tri->b * 255);
                    }
                }
            }
        }
    }
    
    r->triangleCount = 0;
}

void SoftwareRenderer_Present(SoftwareRenderer* r) {
    HDC dc = GetDC(r->windowHandle);
    for (int y = 0; y < r->height; y++) {
        for (int x = 0; x < r->width; x++) {
            SetPixel(r->memoryDC, x, y, r->pixels[y][x]);
        }
    }
    BitBlt(dc, 0, 0, r->width, r->height, r->memoryDC, 0, 0, SRCCOPY);
    ReleaseDC(r->windowHandle, dc);
}