#ifndef ADDX_VISUALIZATION_H
#define ADDX_VISUALIZATION_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_LABEL_LENGTH 128

typedef enum {
    RENDERER_DIRECTX11,
    RENDERER_DIRECTX12,
    RENDERER_VULKAN
} RendererType;

typedef struct {
    float r, g, b, a;
} Color;

typedef struct {
    float x, y;
} Point2D;

typedef struct {
    float x, y, z;
} Point3D;

typedef struct {
    char label[MAX_LABEL_LENGTH];
    float value;
    Color color;
} BarData;

typedef struct {
    char label[MAX_LABEL_LENGTH];
    float value;
    Color color;
} LineData;

typedef struct {
    float value;
    Color color;
    char label[MAX_LABEL_LENGTH];
} PieSlice;

typedef struct {
    float x, y;
    float size;
    Color color;
} ScatterPoint;

typedef struct {
    float minX, maxX;
    float minY, maxY;
    char xLabel[MAX_LABEL_LENGTH];
    char yLabel[MAX_LABEL_LENGTH];
    bool showGrid;
    bool showTicks;
    Color axisColor;
    Color gridColor;
    Color textColor;
} AxisConfig;

typedef struct {
    float width;
    float height;
    float x, y;
    char title[MAX_LABEL_LENGTH];
    Color backgroundColor;
    Color titleColor;
    float titleSize;
    bool showLegend;
} ChartConfig;

typedef struct ChartRenderer ChartRenderer;

struct ChartRendererOps {
    void (*beginFrame)(ChartRenderer*);
    void (*endFrame)(ChartRenderer*);
    void (*drawBarChart)(ChartRenderer*, BarData* data, int dataCount, ChartConfig* config, AxisConfig* axis);
    void (*drawLineChart)(ChartRenderer*, LineData* data, int dataCount, ChartConfig* config, AxisConfig* axis);
    void (*drawPieChart)(ChartRenderer*, PieSlice* data, int dataCount, ChartConfig* config);
    void (*drawScatterPlot)(ChartRenderer*, ScatterPoint* data, int dataCount, ChartConfig* config, AxisConfig* axis);
    void (*drawAreaChart)(ChartRenderer*, LineData* data, int dataCount, ChartConfig* config, AxisConfig* axis);
    void (*drawHistogram)(ChartRenderer*, float* values, int valueCount, int bins, ChartConfig* config, AxisConfig* axis);
    void (*drawBoxPlot)(ChartRenderer*, float* values, int valueCount, ChartConfig* config, AxisConfig* axis);
    void (*drawHeatmap)(ChartRenderer*, float* values, int width, int height, ChartConfig* config);
    void (*drawText)(ChartRenderer*, const char* text, float x, float y, float size, Color color);
    void (*drawLine)(ChartRenderer*, float x1, float y1, float x2, float y2, float thickness, Color color);
    void (*drawCircle)(ChartRenderer*, float x, float y, float radius, Color color, bool filled);
    void (*drawRect)(ChartRenderer*, float x, float y, float width, float height, Color color, bool filled);
    void (*drawPolygon)(ChartRenderer*, Point2D* points, int pointCount, Color color, bool filled);
    void (*draw3DBarChart)(ChartRenderer*, BarData* data, int dataCount, ChartConfig* config, AxisConfig* axis);
    void (*draw3DLineChart)(ChartRenderer*, LineData* data, int dataCount, ChartConfig* config, AxisConfig* axis);
    void (*draw3DScatterPlot)(ChartRenderer*, Point3D* data, int dataCount, ChartConfig* config, AxisConfig* axis);
    void (*drawSurfacePlot)(ChartRenderer*, float* values, int width, int height, ChartConfig* config, AxisConfig* axis);
    void (*destroy)(ChartRenderer*);
};

struct ChartRenderer {
    const struct ChartRendererOps* ops;
};

void Color_Init(Color* c, float r, float g, float b, float a);
Color Color_Red(void);
Color Color_Green(void);
Color Color_Blue(void);
Color Color_Yellow(void);
Color Color_Cyan(void);
Color Color_Magenta(void);
Color Color_White(void);
Color Color_Black(void);
Color Color_Gray(float v);
Color Color_Random(void);

void AxisConfig_Init(AxisConfig* a);
void ChartConfig_Init(ChartConfig* c);

ChartRenderer* VisualizationFactory_CreateRenderer(RendererType type, void* windowHandle, uint32_t width, uint32_t height);
void VisualizationFactory_DestroyRenderer(ChartRenderer* renderer);

#endif