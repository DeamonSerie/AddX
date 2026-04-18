#include "AddXVisualization.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void Color_Init(Color* c, float r, float g, float b, float a) {
    c->r = r;
    c->g = g;
    c->b = b;
    c->a = a;
}

Color Color_Red(void) {
    Color c = {1.0f, 0.0f, 0.0f, 1.0f};
    return c;
}

Color Color_Green(void) {
    Color c = {0.0f, 1.0f, 0.0f, 1.0f};
    return c;
}

Color Color_Blue(void) {
    Color c = {0.0f, 0.0f, 1.0f, 1.0f};
    return c;
}

Color Color_Yellow(void) {
    Color c = {1.0f, 1.0f, 0.0f, 1.0f};
    return c;
}

Color Color_Cyan(void) {
    Color c = {0.0f, 1.0f, 1.0f, 1.0f};
    return c;
}

Color Color_Magenta(void) {
    Color c = {1.0f, 0.0f, 1.0f, 1.0f};
    return c;
}

Color Color_White(void) {
    Color c = {1.0f, 1.0f, 1.0f, 1.0f};
    return c;
}

Color Color_Black(void) {
    Color c = {0.0f, 0.0f, 0.0f, 1.0f};
    return c;
}

Color Color_Gray(float v) {
    Color c = {v, v, v, 1.0f};
    return c;
}

Color Color_Random(void) {
    Color c;
    c.r = (float)rand() / RAND_MAX;
    c.g = (float)rand() / RAND_MAX;
    c.b = (float)rand() / RAND_MAX;
    c.a = 1.0f;
    return c;
}

void AxisConfig_Init(AxisConfig* a) {
    a->minX = 0;
    a->maxX = 100;
    a->minY = 0;
    a->maxY = 100;
    strcpy(a->xLabel, "X");
    strcpy(a->yLabel, "Y");
    a->showGrid = true;
    a->showTicks = true;
    a->axisColor = Color_White();
    a->gridColor = Color_Gray(0.3f);
    a->textColor = Color_White();
}

void ChartConfig_Init(ChartConfig* c) {
    c->width = 400;
    c->height = 300;
    c->x = 0;
    c->y = 0;
    strcpy(c->title, "Chart");
    c->backgroundColor = Color_Gray(0.1f);
    c->titleColor = Color_White();
    c->titleSize = 24;
    c->showLegend = true;
}

typedef struct {
    ChartRenderer base;
    RendererType type;
    void* windowHandle;
    uint32_t width;
    uint32_t height;
} GenericChartRenderer;

static void Generic_BeginFrame(ChartRenderer* r) {}
static void Generic_EndFrame(ChartRenderer* r) {}

static void Generic_DrawBarChart(ChartRenderer* r, BarData* data, int dataCount, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing bar chart with %d bars\n", dataCount);
}

static void Generic_DrawLineChart(ChartRenderer* r, LineData* data, int dataCount, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing line chart with %d points\n", dataCount);
}

static void Generic_DrawPieChart(ChartRenderer* r, PieSlice* data, int dataCount, ChartConfig* config) {
    printf("Drawing pie chart with %d slices\n", dataCount);
}

static void Generic_DrawScatterPlot(ChartRenderer* r, ScatterPoint* data, int dataCount, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing scatter plot with %d points\n", dataCount);
}

static void Generic_DrawAreaChart(ChartRenderer* r, LineData* data, int dataCount, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing area chart\n");
}

static void Generic_DrawHistogram(ChartRenderer* r, float* values, int valueCount, int bins, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing histogram with %d bins\n", bins);
}

static void Generic_DrawBoxPlot(ChartRenderer* r, float* values, int valueCount, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing box plot\n");
}

static void Generic_DrawHeatmap(ChartRenderer* r, float* values, int width, int height, ChartConfig* config) {
    printf("Drawing heatmap %dx%d\n", width, height);
}

static void Generic_DrawText(ChartRenderer* r, const char* text, float x, float y, float size, Color color) {
    printf("Draw text: %s at (%.2f, %.2f)\n", text, x, y);
}

static void Generic_DrawLine(ChartRenderer* r, float x1, float y1, float x2, float y2, float thickness, Color color) {
    printf("Draw line from (%.2f, %.2f) to (%.2f, %.2f)\n", x1, y1, x2, y2);
}

static void Generic_DrawCircle(ChartRenderer* r, float x, float y, float radius, Color color, bool filled) {
    printf("Draw circle at (%.2f, %.2f) radius %.2f\n", x, y, radius);
}

static void Generic_DrawRect(ChartRenderer* r, float x, float y, float width, float height, Color color, bool filled) {
    printf("Draw rect at (%.2f, %.2f) size %.2fx%.2f\n", x, y, width, height);
}

static void Generic_DrawPolygon(ChartRenderer* r, Point2D* points, int pointCount, Color color, bool filled) {
    printf("Draw polygon with %d points\n", pointCount);
}

static void Generic_Draw3DBarChart(ChartRenderer* r, BarData* data, int dataCount, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing 3D bar chart\n");
}

static void Generic_Draw3DLineChart(ChartRenderer* r, LineData* data, int dataCount, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing 3D line chart\n");
}

static void Generic_Draw3DScatterPlot(ChartRenderer* r, Point3D* data, int dataCount, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing 3D scatter plot\n");
}

static void Generic_DrawSurfacePlot(ChartRenderer* r, float* values, int width, int height, ChartConfig* config, AxisConfig* axis) {
    printf("Drawing surface plot %dx%d\n", width, height);
}

static void Generic_Destroy(ChartRenderer* r) {
    free(r);
}

static struct ChartRendererOps genericOps = {
    Generic_BeginFrame,
    Generic_EndFrame,
    Generic_DrawBarChart,
    Generic_DrawLineChart,
    Generic_DrawPieChart,
    Generic_DrawScatterPlot,
    Generic_DrawAreaChart,
    Generic_DrawHistogram,
    Generic_DrawBoxPlot,
    Generic_DrawHeatmap,
    Generic_DrawText,
    Generic_DrawLine,
    Generic_DrawCircle,
    Generic_DrawRect,
    Generic_DrawPolygon,
    Generic_Draw3DBarChart,
    Generic_Draw3DLineChart,
    Generic_Draw3DScatterPlot,
    Generic_DrawSurfacePlot,
    Generic_Destroy
};

ChartRenderer* VisualizationFactory_CreateRenderer(RendererType type, void* windowHandle, uint32_t width, uint32_t height) {
    GenericChartRenderer* renderer = (GenericChartRenderer*)malloc(sizeof(GenericChartRenderer));
    if (!renderer) return NULL;
    
    renderer->base.ops = &genericOps;
    renderer->type = type;
    renderer->windowHandle = windowHandle;
    renderer->width = width;
    renderer->height = height;
    
    printf("Created %s renderer (%dx%d)\n", 
           type == RENDERER_DIRECTX11 ? "DirectX11" : 
           type == RENDERER_DIRECTX12 ? "DirectX12" : "Vulkan",
           width, height);
    
    return (ChartRenderer*)renderer;
}

void VisualizationFactory_DestroyRenderer(ChartRenderer* renderer) {
    if (renderer && renderer->ops && renderer->ops->destroy) {
        renderer->ops->destroy(renderer);
    }
}