#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
typedef struct { float x, y; } Vector2;
typedef struct { float x, y, z; } Vector3;
typedef struct { float x, y, z, w; } Vector4;
typedef struct { float m0, m4, m8, m12, m1, m5, m9, m13, m2, m6, m10, m14, m3, m7, m11, m15; } Matrix;
typedef struct { unsigned char r, g, b, a; } Color;
typedef struct { float x, y, width, height; } Rectangle;
typedef struct { void *data; int width, height, mipmaps, format; } Image;
typedef struct { unsigned int id; int width, height, mipmaps, format; } Texture;
typedef struct { unsigned int id; Texture texture; Texture depth; } RenderTexture;
typedef struct { Rectangle source; int left, top, right, bottom, layout; } NPatchInfo;
typedef struct { int value, offsetX, offsetY, advanceX; Image image; } GlyphInfo;
typedef struct { int baseSize, glyphCount, glyphPadding; Texture texture; Rectangle *recs; GlyphInfo *glyphs; } Font;
typedef struct { Vector3 position, target, up; float fovy; int projection; } Camera3D;
typedef struct { Vector2 offset, target; float rotation, zoom; } Camera2D;
typedef struct { int vertexCount, triangleCount; float *vertices, *texcoords, *texcoords2, *normals, *tangents; unsigned char *colors; unsigned short *indices; float *animVertices, *animNormals; unsigned char *boneIds; float *boneWeights; Matrix *boneMatrices; int boneCount; unsigned int vaoId; unsigned int *vboId; } Mesh;
typedef struct { unsigned int id; int *locs; } Shader;
typedef struct { Texture texture; Color color; float value; } MaterialMap;
typedef struct { Shader shader; MaterialMap *maps; float params[4]; } Material;
typedef struct { Vector3 translation; Vector4 rotation; Vector3 scale; } Transform;
typedef struct { char name[32]; int parent; } BoneInfo;
typedef struct { Matrix transform; int meshCount, materialCount; Mesh *meshes; Material *materials; int *meshMaterial; int boneCount; BoneInfo *bones; Transform *bindPose; } Model;
typedef struct { int boneCount, frameCount; BoneInfo *bones; Transform **framePoses; char name[32]; } ModelAnimation;
typedef struct { Vector3 position, direction; } Ray;
typedef struct { bool hit; float distance; Vector3 point, normal; } RayCollision;
typedef struct { Vector3 min, max; } BoundingBox;
typedef struct { unsigned int frameCount, sampleRate, sampleSize, channels; void *data; } Wave;
typedef struct { void *buffer; void *processor; unsigned int sampleRate, sampleSize, channels; } AudioStream;
typedef struct { AudioStream stream; unsigned int frameCount; } Sound;
typedef struct { AudioStream stream; unsigned int frameCount; bool looping; int ctxType; void *ctxData; } Music;
typedef struct { unsigned int capacity, count; char **paths; } FilePathList;

long g_calls = 0;
long salam_mock_calls(void) { return g_calls; }
double g_last = 0;
double salam_mock_last(void) { return g_last; }
void InitWindow(int width, int height, const char * title) { g_calls++; (void)width; (void)height; (void)title; }
void CloseWindow(void) { g_calls++; }
bool WindowShouldClose(void) { static int n = 0; g_calls++; n++; return n > 3; }
bool IsWindowReady(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
bool IsWindowFullscreen(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
bool IsWindowHidden(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
bool IsWindowMinimized(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
bool IsWindowMaximized(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
bool IsWindowFocused(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
bool IsWindowResized(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
bool IsWindowState(unsigned int flag) { g_calls++; (void)flag; bool z; memset(&z, 0, sizeof z); return z; }
void SetWindowState(unsigned int flags) { g_calls++; (void)flags; }
void ClearWindowState(unsigned int flags) { g_calls++; (void)flags; }
void ToggleFullscreen(void) { g_calls++; }
void ToggleBorderlessWindowed(void) { g_calls++; }
void MaximizeWindow(void) { g_calls++; }
void MinimizeWindow(void) { g_calls++; }
void RestoreWindow(void) { g_calls++; }
void SetWindowIcon(Image image) { g_calls++; (void)image; }
void SetWindowTitle(const char * title) { g_calls++; (void)title; }
void SetWindowPosition(int x, int y) { g_calls++; (void)x; (void)y; }
void SetWindowMonitor(int monitor) { g_calls++; (void)monitor; }
void SetWindowMinSize(int width, int height) { g_calls++; (void)width; (void)height; }
void SetWindowMaxSize(int width, int height) { g_calls++; (void)width; (void)height; }
void SetWindowSize(int width, int height) { g_calls++; (void)width; (void)height; }
void SetWindowOpacity(float opacity) { g_calls++; (void)opacity; }
void SetWindowFocused(void) { g_calls++; }
int GetScreenWidth(void) { g_calls++; return 800; }
int GetScreenHeight(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
int GetRenderWidth(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
int GetRenderHeight(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
int GetMonitorCount(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
int GetCurrentMonitor(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
Vector2 GetMonitorPosition(int monitor) { g_calls++; (void)monitor; Vector2 z; memset(&z, 0, sizeof z); return z; }
int GetMonitorWidth(int monitor) { g_calls++; (void)monitor; int z; memset(&z, 0, sizeof z); return z; }
int GetMonitorHeight(int monitor) { g_calls++; (void)monitor; int z; memset(&z, 0, sizeof z); return z; }
int GetMonitorPhysicalWidth(int monitor) { g_calls++; (void)monitor; int z; memset(&z, 0, sizeof z); return z; }
int GetMonitorPhysicalHeight(int monitor) { g_calls++; (void)monitor; int z; memset(&z, 0, sizeof z); return z; }
int GetMonitorRefreshRate(int monitor) { g_calls++; (void)monitor; int z; memset(&z, 0, sizeof z); return z; }
Vector2 GetWindowPosition(void) { g_calls++; Vector2 z; memset(&z, 0, sizeof z); return z; }
Vector2 GetWindowScaleDPI(void) { g_calls++; Vector2 z; memset(&z, 0, sizeof z); return z; }
const char *GetMonitorName(int m) { g_calls++; return m == 0 ? "primary" : "other"; }
void SetClipboardText(const char * text) { g_calls++; (void)text; }
const char * GetClipboardText(void) { g_calls++; return 0; }
void EnableEventWaiting(void) { g_calls++; }
void DisableEventWaiting(void) { g_calls++; }
void ShowCursor(void) { g_calls++; }
void HideCursor(void) { g_calls++; }
bool IsCursorHidden(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
void EnableCursor(void) { g_calls++; }
void DisableCursor(void) { g_calls++; }
bool IsCursorOnScreen(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
void ClearBackground(Color color) { g_calls++; (void)color; }
void BeginDrawing(void) { g_calls++; }
void EndDrawing(void) { g_calls++; }
void BeginMode2D(Camera2D camera) { g_calls++; (void)camera; }
void EndMode2D(void) { g_calls++; }
void BeginMode3D(Camera3D camera) { g_calls++; (void)camera; }
void EndMode3D(void) { g_calls++; }
void BeginTextureMode(RenderTexture target) { g_calls++; (void)target; }
void EndTextureMode(void) { g_calls++; }
void BeginShaderMode(Shader shader) { g_calls++; (void)shader; }
void EndShaderMode(void) { g_calls++; }
void BeginBlendMode(int mode) { g_calls++; (void)mode; }
void EndBlendMode(void) { g_calls++; }
void BeginScissorMode(int x, int y, int width, int height) { g_calls++; (void)x; (void)y; (void)width; (void)height; }
void EndScissorMode(void) { g_calls++; }
Shader LoadShader(const char * vsFileName, const char * fsFileName) { g_calls++; (void)vsFileName; (void)fsFileName; Shader z; memset(&z, 0, sizeof z); return z; }
Shader LoadShaderFromMemory(const char * vsCode, const char * fsCode) { g_calls++; (void)vsCode; (void)fsCode; Shader z; memset(&z, 0, sizeof z); return z; }
bool IsShaderValid(Shader shader) { g_calls++; (void)shader; bool z; memset(&z, 0, sizeof z); return z; }
int GetShaderLocation(Shader shader, const char * uniformName) { g_calls++; (void)shader; (void)uniformName; int z; memset(&z, 0, sizeof z); return z; }
int GetShaderLocationAttrib(Shader shader, const char * attribName) { g_calls++; (void)shader; (void)attribName; int z; memset(&z, 0, sizeof z); return z; }
void SetShaderValue(Shader shader, int locIndex, void * value, int uniformType) { g_calls++; (void)shader; (void)locIndex; (void)value; (void)uniformType; }
void SetShaderValueV(Shader shader, int locIndex, void * value, int uniformType, int count) { g_calls++; (void)shader; (void)locIndex; (void)value; (void)uniformType; (void)count; }
void SetShaderValueMatrix(Shader sh, int loc, Matrix m) { g_calls++; g_last = sh.id + loc + m.m15; }
void SetShaderValueTexture(Shader shader, int locIndex, Texture texture) { g_calls++; (void)shader; (void)locIndex; (void)texture; }
void UnloadShader(Shader shader) { g_calls++; (void)shader; }
Ray GetScreenToWorldRay(Vector2 position, Camera3D camera) { g_calls++; (void)position; (void)camera; Ray z; memset(&z, 0, sizeof z); return z; }
Ray GetScreenToWorldRayEx(Vector2 position, Camera3D camera, int width, int height) { g_calls++; (void)position; (void)camera; (void)width; (void)height; Ray z; memset(&z, 0, sizeof z); return z; }
Vector2 GetWorldToScreen(Vector3 position, Camera3D camera) { g_calls++; (void)position; (void)camera; Vector2 z; memset(&z, 0, sizeof z); return z; }
Vector2 GetWorldToScreenEx(Vector3 position, Camera3D camera, int width, int height) { g_calls++; (void)position; (void)camera; (void)width; (void)height; Vector2 z; memset(&z, 0, sizeof z); return z; }
Vector2 GetWorldToScreen2D(Vector2 position, Camera2D camera) { g_calls++; (void)position; (void)camera; Vector2 z; memset(&z, 0, sizeof z); return z; }
Vector2 GetScreenToWorld2D(Vector2 position, Camera2D camera) { g_calls++; (void)position; (void)camera; Vector2 z; memset(&z, 0, sizeof z); return z; }
Matrix GetCameraMatrix(Camera3D camera) { g_calls++; (void)camera; Matrix z; memset(&z, 0, sizeof z); return z; }
Matrix GetCameraMatrix2D(Camera2D c) { g_calls++; Matrix m; memset(&m, 0, sizeof m); m.m0 = c.zoom; m.m5 = c.rotation; m.m12 = c.offset.x; m.m13 = c.target.y; return m; }
void SetTargetFPS(int fps) { g_calls++; (void)fps; }
float GetFrameTime(void) { g_calls++; return 0.5f; }
double GetTime(void) { g_calls++; double z; memset(&z, 0, sizeof z); return z; }
int GetFPS(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
void SwapScreenBuffer(void) { g_calls++; }
void PollInputEvents(void) { g_calls++; }
void WaitTime(double seconds) { g_calls++; (void)seconds; }
void SetRandomSeed(unsigned int seed) { g_calls++; (void)seed; }
int GetRandomValue(int a, int b) { g_calls++; return a + b; }
void TakeScreenshot(const char * fileName) { g_calls++; (void)fileName; }
void SetConfigFlags(unsigned int flags) { g_calls++; (void)flags; }
void OpenURL(const char * url) { g_calls++; (void)url; }
void SetTraceLogLevel(int logLevel) { g_calls++; (void)logLevel; }
void * MemAlloc(unsigned int size) { g_calls++; (void)size; return 0; }
void MemFree(void * ptr) { g_calls++; (void)ptr; }
bool FileExists(const char * fileName) { g_calls++; (void)fileName; bool z; memset(&z, 0, sizeof z); return z; }
bool DirectoryExists(const char * dirPath) { g_calls++; (void)dirPath; bool z; memset(&z, 0, sizeof z); return z; }
bool IsFileExtension(const char * fileName, const char * ext) { g_calls++; (void)fileName; (void)ext; bool z; memset(&z, 0, sizeof z); return z; }
int GetFileLength(const char * fileName) { g_calls++; (void)fileName; int z; memset(&z, 0, sizeof z); return z; }
const char * GetFileExtension(const char * fileName) { g_calls++; (void)fileName; return 0; }
const char * GetFileName(const char * filePath) { g_calls++; (void)filePath; return 0; }
const char * GetFileNameWithoutExt(const char * filePath) { g_calls++; (void)filePath; return 0; }
const char * GetDirectoryPath(const char * filePath) { g_calls++; (void)filePath; return 0; }
const char * GetPrevDirectoryPath(const char * dirPath) { g_calls++; (void)dirPath; return 0; }
const char * GetWorkingDirectory(void) { g_calls++; return 0; }
const char * GetApplicationDirectory(void) { g_calls++; return 0; }
bool ChangeDirectory(const char * dir) { g_calls++; (void)dir; bool z; memset(&z, 0, sizeof z); return z; }
bool IsPathFile(const char * path) { g_calls++; (void)path; bool z; memset(&z, 0, sizeof z); return z; }
bool IsFileNameValid(const char * fileName) { g_calls++; (void)fileName; bool z; memset(&z, 0, sizeof z); return z; }
FilePathList LoadDirectoryFiles(const char * dirPath) { g_calls++; (void)dirPath; FilePathList z; memset(&z, 0, sizeof z); return z; }
void UnloadDirectoryFiles(FilePathList files) { g_calls++; (void)files; }
bool IsFileDropped(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
FilePathList LoadDroppedFiles(void) { g_calls++; FilePathList z; memset(&z, 0, sizeof z); return z; }
void UnloadDroppedFiles(FilePathList files) { g_calls++; (void)files; }
const char * LoadFileText(const char * fileName) { g_calls++; (void)fileName; return 0; }
void UnloadFileText(const char * text) { g_calls++; (void)text; }
bool SaveFileText(const char * fileName, const char * text) { g_calls++; (void)fileName; (void)text; bool z; memset(&z, 0, sizeof z); return z; }
unsigned char *LoadFileData(const char *f, int *n) { g_calls++; (void)f; *n = 42; return 0; }
void UnloadFileData(unsigned char * data) { g_calls++; (void)data; }
bool SaveFileData(const char * fileName, void * data, int dataSize) { g_calls++; (void)fileName; (void)data; (void)dataSize; bool z; memset(&z, 0, sizeof z); return z; }
bool IsKeyPressed(int key) { g_calls++; (void)key; bool z; memset(&z, 0, sizeof z); return z; }
bool IsKeyPressedRepeat(int key) { g_calls++; (void)key; bool z; memset(&z, 0, sizeof z); return z; }
bool IsKeyDown(int k) { g_calls++; return k == 65; }
bool IsKeyReleased(int key) { g_calls++; (void)key; bool z; memset(&z, 0, sizeof z); return z; }
bool IsKeyUp(int key) { g_calls++; (void)key; bool z; memset(&z, 0, sizeof z); return z; }
int GetKeyPressed(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
int GetCharPressed(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
void SetExitKey(int key) { g_calls++; (void)key; }
bool IsGamepadAvailable(int gamepad) { g_calls++; (void)gamepad; bool z; memset(&z, 0, sizeof z); return z; }
const char * GetGamepadName(int gamepad) { g_calls++; (void)gamepad; return 0; }
bool IsGamepadButtonPressed(int gamepad, int button) { g_calls++; (void)gamepad; (void)button; bool z; memset(&z, 0, sizeof z); return z; }
bool IsGamepadButtonDown(int gamepad, int button) { g_calls++; (void)gamepad; (void)button; bool z; memset(&z, 0, sizeof z); return z; }
bool IsGamepadButtonReleased(int gamepad, int button) { g_calls++; (void)gamepad; (void)button; bool z; memset(&z, 0, sizeof z); return z; }
bool IsGamepadButtonUp(int gamepad, int button) { g_calls++; (void)gamepad; (void)button; bool z; memset(&z, 0, sizeof z); return z; }
int GetGamepadButtonPressed(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
int GetGamepadAxisCount(int gamepad) { g_calls++; (void)gamepad; int z; memset(&z, 0, sizeof z); return z; }
float GetGamepadAxisMovement(int gamepad, int axis) { g_calls++; (void)gamepad; (void)axis; float z; memset(&z, 0, sizeof z); return z; }
int SetGamepadMappings(const char * mappings) { g_calls++; (void)mappings; int z; memset(&z, 0, sizeof z); return z; }
void SetGamepadVibration(int gamepad, float leftMotor, float rightMotor, float duration) { g_calls++; (void)gamepad; (void)leftMotor; (void)rightMotor; (void)duration; }
bool IsMouseButtonPressed(int button) { g_calls++; (void)button; bool z; memset(&z, 0, sizeof z); return z; }
bool IsMouseButtonDown(int button) { g_calls++; (void)button; bool z; memset(&z, 0, sizeof z); return z; }
bool IsMouseButtonReleased(int button) { g_calls++; (void)button; bool z; memset(&z, 0, sizeof z); return z; }
bool IsMouseButtonUp(int button) { g_calls++; (void)button; bool z; memset(&z, 0, sizeof z); return z; }
int GetMouseX(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
int GetMouseY(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
Vector2 GetMousePosition(void) { g_calls++; Vector2 v = {3, 4}; return v; }
Vector2 GetMouseDelta(void) { g_calls++; Vector2 z; memset(&z, 0, sizeof z); return z; }
void SetMousePosition(int x, int y) { g_calls++; (void)x; (void)y; }
void SetMouseOffset(int offsetX, int offsetY) { g_calls++; (void)offsetX; (void)offsetY; }
void SetMouseScale(float scaleX, float scaleY) { g_calls++; (void)scaleX; (void)scaleY; }
float GetMouseWheelMove(void) { g_calls++; float z; memset(&z, 0, sizeof z); return z; }
Vector2 GetMouseWheelMoveV(void) { g_calls++; Vector2 z; memset(&z, 0, sizeof z); return z; }
void SetMouseCursor(int cursor) { g_calls++; (void)cursor; }
int GetTouchX(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
int GetTouchY(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
Vector2 GetTouchPosition(int index) { g_calls++; (void)index; Vector2 z; memset(&z, 0, sizeof z); return z; }
int GetTouchPointId(int index) { g_calls++; (void)index; int z; memset(&z, 0, sizeof z); return z; }
int GetTouchPointCount(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
void SetGesturesEnabled(unsigned int flags) { g_calls++; (void)flags; }
bool IsGestureDetected(unsigned int gesture) { g_calls++; (void)gesture; bool z; memset(&z, 0, sizeof z); return z; }
int GetGestureDetected(void) { g_calls++; int z; memset(&z, 0, sizeof z); return z; }
float GetGestureHoldDuration(void) { g_calls++; float z; memset(&z, 0, sizeof z); return z; }
Vector2 GetGestureDragVector(void) { g_calls++; Vector2 z; memset(&z, 0, sizeof z); return z; }
float GetGestureDragAngle(void) { g_calls++; float z; memset(&z, 0, sizeof z); return z; }
Vector2 GetGesturePinchVector(void) { g_calls++; Vector2 z; memset(&z, 0, sizeof z); return z; }
float GetGesturePinchAngle(void) { g_calls++; float z; memset(&z, 0, sizeof z); return z; }
void UpdateCamera(Camera3D *camera, int mode) { g_calls++; (void)camera; (void)mode; }
void UpdateCameraPro(Camera3D *camera, Vector3 movement, Vector3 rotation, float zoom) { g_calls++; (void)camera; (void)movement; (void)rotation; (void)zoom; }
void SetShapesTexture(Texture texture, Rectangle source) { g_calls++; (void)texture; (void)source; }
Texture GetShapesTexture(void) { g_calls++; Texture z; memset(&z, 0, sizeof z); return z; }
Rectangle GetShapesTextureRectangle(void) { g_calls++; Rectangle z; memset(&z, 0, sizeof z); return z; }
void DrawPixel(int posX, int posY, Color color) { g_calls++; (void)posX; (void)posY; (void)color; }
void DrawPixelV(Vector2 position, Color color) { g_calls++; (void)position; (void)color; }
void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color) { g_calls++; (void)startPosX; (void)startPosY; (void)endPosX; (void)endPosY; (void)color; }
void DrawLineV(Vector2 startPos, Vector2 endPos, Color color) { g_calls++; (void)startPos; (void)endPos; (void)color; }
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color) { g_calls++; (void)startPos; (void)endPos; (void)thick; (void)color; }
void DrawLineStrip(Vector2 * points, int pointCount, Color color) { g_calls++; (void)points; (void)pointCount; (void)color; }
void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color) { g_calls++; (void)startPos; (void)endPos; (void)thick; (void)color; }
void DrawCircle(int centerX, int centerY, float radius, Color color) { g_calls++; (void)centerX; (void)centerY; (void)radius; (void)color; }
void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color) { g_calls++; (void)center; (void)radius; (void)startAngle; (void)endAngle; (void)segments; (void)color; }
void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color) { g_calls++; (void)center; (void)radius; (void)startAngle; (void)endAngle; (void)segments; (void)color; }
void DrawCircleGradient(int centerX, int centerY, float radius, Color inner, Color outer) { g_calls++; (void)centerX; (void)centerY; (void)radius; (void)inner; (void)outer; }
void DrawCircleV(Vector2 center, float radius, Color color) { g_calls++; (void)center; (void)radius; (void)color; }
void DrawCircleLines(int centerX, int centerY, float radius, Color color) { g_calls++; (void)centerX; (void)centerY; (void)radius; (void)color; }
void DrawCircleLinesV(Vector2 center, float radius, Color color) { g_calls++; (void)center; (void)radius; (void)color; }
void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color) { g_calls++; (void)centerX; (void)centerY; (void)radiusH; (void)radiusV; (void)color; }
void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color) { g_calls++; (void)centerX; (void)centerY; (void)radiusH; (void)radiusV; (void)color; }
void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color) { g_calls++; (void)center; (void)innerRadius; (void)outerRadius; (void)startAngle; (void)endAngle; (void)segments; (void)color; }
void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color) { g_calls++; (void)center; (void)innerRadius; (void)outerRadius; (void)startAngle; (void)endAngle; (void)segments; (void)color; }
void DrawRectangle(int posX, int posY, int width, int height, Color color) { g_calls++; (void)posX; (void)posY; (void)width; (void)height; (void)color; }
void DrawRectangleV(Vector2 position, Vector2 size, Color color) { g_calls++; (void)position; (void)size; (void)color; }
void DrawRectangleRec(Rectangle rec, Color color) { g_calls++; (void)rec; (void)color; }
void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color) { g_calls++; (void)rec; (void)origin; (void)rotation; (void)color; }
void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom) { g_calls++; (void)posX; (void)posY; (void)width; (void)height; (void)top; (void)bottom; }
void DrawRectangleGradientH(int posX, int posY, int width, int height, Color left, Color right) { g_calls++; (void)posX; (void)posY; (void)width; (void)height; (void)left; (void)right; }
void DrawRectangleGradientEx(Rectangle rec, Color topLeft, Color bottomLeft, Color topRight, Color bottomRight) { g_calls++; (void)rec; (void)topLeft; (void)bottomLeft; (void)topRight; (void)bottomRight; }
void DrawRectangleLines(int posX, int posY, int width, int height, Color color) { g_calls++; (void)posX; (void)posY; (void)width; (void)height; (void)color; }
void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color) { g_calls++; (void)rec; (void)lineThick; (void)color; }
void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color) { g_calls++; (void)rec; (void)roundness; (void)segments; (void)color; }
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color) { g_calls++; (void)v1; (void)v2; (void)v3; (void)color; }
void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color) { g_calls++; (void)v1; (void)v2; (void)v3; (void)color; }
void DrawTriangleFan(Vector2 * points, int pointCount, Color color) { g_calls++; (void)points; (void)pointCount; (void)color; }
void DrawTriangleStrip(Vector2 * points, int pointCount, Color color) { g_calls++; (void)points; (void)pointCount; (void)color; }
void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color) { g_calls++; (void)center; (void)sides; (void)radius; (void)rotation; (void)color; }
void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color) { g_calls++; (void)center; (void)sides; (void)radius; (void)rotation; (void)color; }
void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick, Color color) { g_calls++; (void)center; (void)sides; (void)radius; (void)rotation; (void)lineThick; (void)color; }
void DrawSplineLinear(Vector2 * points, int pointCount, float thick, Color color) { g_calls++; (void)points; (void)pointCount; (void)thick; (void)color; }
void DrawSplineBasis(Vector2 * points, int pointCount, float thick, Color color) { g_calls++; (void)points; (void)pointCount; (void)thick; (void)color; }
void DrawSplineCatmullRom(Vector2 * points, int pointCount, float thick, Color color) { g_calls++; (void)points; (void)pointCount; (void)thick; (void)color; }
void DrawSplineBezierQuadratic(Vector2 * points, int pointCount, float thick, Color color) { g_calls++; (void)points; (void)pointCount; (void)thick; (void)color; }
void DrawSplineBezierCubic(Vector2 * points, int pointCount, float thick, Color color) { g_calls++; (void)points; (void)pointCount; (void)thick; (void)color; }
bool CheckCollisionRecs(Rectangle a, Rectangle b) { g_calls++; return a.x < b.x + b.width && a.x + a.width > b.x && a.y < b.y + b.height && a.y + a.height > b.y; }
bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2) { g_calls++; (void)center1; (void)radius1; (void)center2; (void)radius2; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec) { g_calls++; (void)center; (void)radius; (void)rec; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionPointRec(Vector2 point, Rectangle rec) { g_calls++; (void)point; (void)rec; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius) { g_calls++; (void)point; (void)center; (void)radius; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3) { g_calls++; (void)point; (void)p1; (void)p2; (void)p3; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionPointLine(Vector2 point, Vector2 p1, Vector2 p2, int threshold) { g_calls++; (void)point; (void)p1; (void)p2; (void)threshold; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionPointPoly(Vector2 point, Vector2 * points, int pointCount) { g_calls++; (void)point; (void)points; (void)pointCount; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionLines(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Vector2 *p) { g_calls++; (void)a; (void)b; (void)c; (void)d; p->x = 7; p->y = 8; return true; }
Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2) { g_calls++; (void)rec1; (void)rec2; Rectangle z; memset(&z, 0, sizeof z); return z; }
Image LoadImage(const char * fileName) { g_calls++; (void)fileName; Image z; memset(&z, 0, sizeof z); return z; }
Image LoadImageRaw(const char * fileName, int width, int height, int format, int headerSize) { g_calls++; (void)fileName; (void)width; (void)height; (void)format; (void)headerSize; Image z; memset(&z, 0, sizeof z); return z; }
Image LoadImageFromMemory(const char * fileType, unsigned char * fileData, int dataSize) { g_calls++; (void)fileType; (void)fileData; (void)dataSize; Image z; memset(&z, 0, sizeof z); return z; }
Image LoadImageFromTexture(Texture texture) { g_calls++; (void)texture; Image z; memset(&z, 0, sizeof z); return z; }
Image LoadImageFromScreen(void) { g_calls++; Image z; memset(&z, 0, sizeof z); return z; }
bool IsImageValid(Image image) { g_calls++; (void)image; bool z; memset(&z, 0, sizeof z); return z; }
void UnloadImage(Image image) { g_calls++; (void)image; }
bool ExportImage(Image image, const char * fileName) { g_calls++; (void)image; (void)fileName; bool z; memset(&z, 0, sizeof z); return z; }
Image GenImageColor(int w, int h, Color c) { g_calls++; Image i = {0, w, h, c.r, c.a}; return i; }
Image GenImageGradientLinear(int width, int height, int direction, Color startColor, Color endColor) { g_calls++; (void)width; (void)height; (void)direction; (void)startColor; (void)endColor; Image z; memset(&z, 0, sizeof z); return z; }
Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer) { g_calls++; (void)width; (void)height; (void)density; (void)inner; (void)outer; Image z; memset(&z, 0, sizeof z); return z; }
Image GenImageGradientSquare(int width, int height, float density, Color inner, Color outer) { g_calls++; (void)width; (void)height; (void)density; (void)inner; (void)outer; Image z; memset(&z, 0, sizeof z); return z; }
Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2) { g_calls++; (void)width; (void)height; (void)checksX; (void)checksY; (void)col1; (void)col2; Image z; memset(&z, 0, sizeof z); return z; }
Image GenImageWhiteNoise(int width, int height, float factor) { g_calls++; (void)width; (void)height; (void)factor; Image z; memset(&z, 0, sizeof z); return z; }
Image GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale) { g_calls++; (void)width; (void)height; (void)offsetX; (void)offsetY; (void)scale; Image z; memset(&z, 0, sizeof z); return z; }
Image GenImageCellular(int width, int height, int tileSize) { g_calls++; (void)width; (void)height; (void)tileSize; Image z; memset(&z, 0, sizeof z); return z; }
Image ImageCopy(Image image) { g_calls++; (void)image; Image z; memset(&z, 0, sizeof z); return z; }
Image ImageFromImage(Image image, Rectangle rec) { g_calls++; (void)image; (void)rec; Image z; memset(&z, 0, sizeof z); return z; }
Image ImageText(const char * text, int fontSize, Color color) { g_calls++; (void)text; (void)fontSize; (void)color; Image z; memset(&z, 0, sizeof z); return z; }
void ImageFormat(Image *image, int newFormat) { g_calls++; (void)image; (void)newFormat; }
void ImageToPOT(Image *image, Color fill) { g_calls++; (void)image; (void)fill; }
void ImageCrop(Image *image, Rectangle crop) { g_calls++; (void)image; (void)crop; }
void ImageAlphaCrop(Image *image, float threshold) { g_calls++; (void)image; (void)threshold; }
void ImageAlphaClear(Image *image, Color color, float threshold) { g_calls++; (void)image; (void)color; (void)threshold; }
void ImageAlphaMask(Image *image, Image alphaMask) { g_calls++; (void)image; (void)alphaMask; }
void ImageAlphaPremultiply(Image *image) { g_calls++; (void)image; }
void ImageBlurGaussian(Image *image, int blurSize) { g_calls++; (void)image; (void)blurSize; }
void ImageResize(Image *i, int w, int h) { g_calls++; i->width = w; i->height = h; }
void ImageResizeNN(Image *image, int newWidth, int newHeight) { g_calls++; (void)image; (void)newWidth; (void)newHeight; }
void ImageResizeCanvas(Image *image, int newWidth, int newHeight, int offsetX, int offsetY, Color fill) { g_calls++; (void)image; (void)newWidth; (void)newHeight; (void)offsetX; (void)offsetY; (void)fill; }
void ImageMipmaps(Image *image) { g_calls++; (void)image; }
void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp) { g_calls++; (void)image; (void)rBpp; (void)gBpp; (void)bBpp; (void)aBpp; }
void ImageFlipVertical(Image *image) { g_calls++; (void)image; }
void ImageFlipHorizontal(Image *image) { g_calls++; (void)image; }
void ImageRotate(Image *image, int degrees) { g_calls++; (void)image; (void)degrees; }
void ImageRotateCW(Image *image) { g_calls++; (void)image; }
void ImageRotateCCW(Image *image) { g_calls++; (void)image; }
void ImageColorTint(Image *image, Color color) { g_calls++; (void)image; (void)color; }
void ImageColorInvert(Image *image) { g_calls++; (void)image; }
void ImageColorGrayscale(Image *image) { g_calls++; (void)image; }
void ImageColorContrast(Image *image, float contrast) { g_calls++; (void)image; (void)contrast; }
void ImageColorBrightness(Image *image, int brightness) { g_calls++; (void)image; (void)brightness; }
void ImageColorReplace(Image *image, Color color, Color replace) { g_calls++; (void)image; (void)color; (void)replace; }
Color * LoadImageColors(Image image) { g_calls++; (void)image; return 0; }
void UnloadImageColors(Color * colors) { g_calls++; (void)colors; }
Rectangle GetImageAlphaBorder(Image image, float threshold) { g_calls++; (void)image; (void)threshold; Rectangle z; memset(&z, 0, sizeof z); return z; }
Color GetImageColor(Image image, int x, int y) { g_calls++; (void)image; (void)x; (void)y; Color z; memset(&z, 0, sizeof z); return z; }
void ImageClearBackground(Image *dst, Color color) { g_calls++; (void)dst; (void)color; }
void ImageDrawPixel(Image *dst, int posX, int posY, Color color) { g_calls++; (void)dst; (void)posX; (void)posY; (void)color; }
void ImageDrawPixelV(Image *dst, Vector2 position, Color color) { g_calls++; (void)dst; (void)position; (void)color; }
void ImageDrawLine(Image *dst, int startPosX, int startPosY, int endPosX, int endPosY, Color color) { g_calls++; (void)dst; (void)startPosX; (void)startPosY; (void)endPosX; (void)endPosY; (void)color; }
void ImageDrawLineV(Image *dst, Vector2 startPos, Vector2 endPos, Color color) { g_calls++; (void)dst; (void)startPos; (void)endPos; (void)color; }
void ImageDrawCircle(Image *dst, int centerX, int centerY, int radius, Color color) { g_calls++; (void)dst; (void)centerX; (void)centerY; (void)radius; (void)color; }
void ImageDrawCircleV(Image *dst, Vector2 center, int radius, Color color) { g_calls++; (void)dst; (void)center; (void)radius; (void)color; }
void ImageDrawRectangle(Image *dst, int posX, int posY, int width, int height, Color color) { g_calls++; (void)dst; (void)posX; (void)posY; (void)width; (void)height; (void)color; }
void ImageDrawRectangleV(Image *dst, Vector2 position, Vector2 size, Color color) { g_calls++; (void)dst; (void)position; (void)size; (void)color; }
void ImageDrawRectangleRec(Image *dst, Rectangle rec, Color color) { g_calls++; (void)dst; (void)rec; (void)color; }
void ImageDrawRectangleLines(Image *dst, Rectangle rec, int thick, Color color) { g_calls++; (void)dst; (void)rec; (void)thick; (void)color; }
void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec, Color tint) { g_calls++; (void)dst; (void)src; (void)srcRec; (void)dstRec; (void)tint; }
void ImageDrawText(Image *dst, const char * text, int posX, int posY, int fontSize, Color color) { g_calls++; (void)dst; (void)text; (void)posX; (void)posY; (void)fontSize; (void)color; }
Texture LoadTexture(const char *f) { g_calls++; Texture t = {1, 64, 32, 1, 7}; (void)f; return t; }
Texture LoadTextureFromImage(Image image) { g_calls++; (void)image; Texture z; memset(&z, 0, sizeof z); return z; }
Texture LoadTextureCubemap(Image image, int cubeLayout) { g_calls++; (void)image; (void)cubeLayout; Texture z; memset(&z, 0, sizeof z); return z; }
RenderTexture LoadRenderTexture(int width, int height) { g_calls++; (void)width; (void)height; RenderTexture z; memset(&z, 0, sizeof z); return z; }
bool IsTextureValid(Texture texture) { g_calls++; (void)texture; bool z; memset(&z, 0, sizeof z); return z; }
void UnloadTexture(Texture texture) { g_calls++; (void)texture; }
bool IsRenderTextureValid(RenderTexture target) { g_calls++; (void)target; bool z; memset(&z, 0, sizeof z); return z; }
void UnloadRenderTexture(RenderTexture target) { g_calls++; (void)target; }
void UpdateTexture(Texture texture, void * pixels) { g_calls++; (void)texture; (void)pixels; }
void UpdateTextureRec(Texture texture, Rectangle rec, void * pixels) { g_calls++; (void)texture; (void)rec; (void)pixels; }
void GenTextureMipmaps(Texture *texture) { g_calls++; (void)texture; }
void SetTextureFilter(Texture texture, int filter) { g_calls++; (void)texture; (void)filter; }
void SetTextureWrap(Texture texture, int wrap) { g_calls++; (void)texture; (void)wrap; }
void DrawTexture(Texture texture, int posX, int posY, Color tint) { g_calls++; (void)texture; (void)posX; (void)posY; (void)tint; }
void DrawTextureV(Texture texture, Vector2 position, Color tint) { g_calls++; (void)texture; (void)position; (void)tint; }
void DrawTextureEx(Texture texture, Vector2 position, float rotation, float scale, Color tint) { g_calls++; (void)texture; (void)position; (void)rotation; (void)scale; (void)tint; }
void DrawTextureRec(Texture texture, Rectangle source, Vector2 position, Color tint) { g_calls++; (void)texture; (void)source; (void)position; (void)tint; }
void DrawTexturePro(Texture t, Rectangle s, Rectangle d, Vector2 o, float r, Color c) { g_calls++; g_last = t.width + s.width + d.height + o.y + r + c.b; }
void DrawTextureNPatch(Texture texture, NPatchInfo nPatchInfo, Rectangle dest, Vector2 origin, float rotation, Color tint) { g_calls++; (void)texture; (void)nPatchInfo; (void)dest; (void)origin; (void)rotation; (void)tint; }
bool ColorIsEqual(Color a, Color b) { g_calls++; return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }
Color Fade(Color c, float alpha) { g_calls++; c.a = (unsigned char)(alpha * 255.0f); return c; }
int ColorToInt(Color c) { g_calls++; return (c.r << 24) | (c.g << 16) | (c.b << 8) | c.a; }
Vector4 ColorNormalize(Color color) { g_calls++; (void)color; Vector4 z; memset(&z, 0, sizeof z); return z; }
Color ColorFromNormalized(Vector4 normalized) { g_calls++; (void)normalized; Color z; memset(&z, 0, sizeof z); return z; }
Vector3 ColorToHSV(Color color) { g_calls++; (void)color; Vector3 z; memset(&z, 0, sizeof z); return z; }
Color ColorFromHSV(float hue, float saturation, float value) { g_calls++; (void)hue; (void)saturation; (void)value; Color z; memset(&z, 0, sizeof z); return z; }
Color ColorTint(Color color, Color tint) { g_calls++; (void)color; (void)tint; Color z; memset(&z, 0, sizeof z); return z; }
Color ColorBrightness(Color color, float factor) { g_calls++; (void)color; (void)factor; Color z; memset(&z, 0, sizeof z); return z; }
Color ColorContrast(Color color, float contrast) { g_calls++; (void)color; (void)contrast; Color z; memset(&z, 0, sizeof z); return z; }
Color ColorAlpha(Color color, float alpha) { g_calls++; (void)color; (void)alpha; Color z; memset(&z, 0, sizeof z); return z; }
Color ColorAlphaBlend(Color dst, Color src, Color tint) { g_calls++; (void)dst; (void)src; (void)tint; Color z; memset(&z, 0, sizeof z); return z; }
Color ColorLerp(Color color1, Color color2, float factor) { g_calls++; (void)color1; (void)color2; (void)factor; Color z; memset(&z, 0, sizeof z); return z; }
Color GetColor(unsigned int hexValue) { g_calls++; (void)hexValue; Color z; memset(&z, 0, sizeof z); return z; }
Color GetPixelColor(void * srcPtr, int format) { g_calls++; (void)srcPtr; (void)format; Color z; memset(&z, 0, sizeof z); return z; }
void SetPixelColor(void * dstPtr, Color color, int format) { g_calls++; (void)dstPtr; (void)color; (void)format; }
int GetPixelDataSize(int width, int height, int format) { g_calls++; (void)width; (void)height; (void)format; int z; memset(&z, 0, sizeof z); return z; }
Font GetFontDefault(void) { g_calls++; Font f; memset(&f, 0, sizeof f); f.baseSize = 10; f.glyphCount = 95; return f; }
Font LoadFont(const char * fileName) { g_calls++; (void)fileName; Font z; memset(&z, 0, sizeof z); return z; }
Font LoadFontEx(const char * fileName, int fontSize, int * codepoints, int codepointCount) { g_calls++; (void)fileName; (void)fontSize; (void)codepoints; (void)codepointCount; Font z; memset(&z, 0, sizeof z); return z; }
Font LoadFontFromImage(Image image, Color key, int firstChar) { g_calls++; (void)image; (void)key; (void)firstChar; Font z; memset(&z, 0, sizeof z); return z; }
bool IsFontValid(Font font) { g_calls++; (void)font; bool z; memset(&z, 0, sizeof z); return z; }
void UnloadFont(Font font) { g_calls++; (void)font; }
void DrawFPS(int posX, int posY) { g_calls++; (void)posX; (void)posY; }
void DrawText(const char * text, int posX, int posY, int fontSize, Color color) { g_calls++; (void)text; (void)posX; (void)posY; (void)fontSize; (void)color; }
void DrawTextEx(Font font, const char * text, Vector2 position, float fontSize, float spacing, Color tint) { g_calls++; (void)font; (void)text; (void)position; (void)fontSize; (void)spacing; (void)tint; }
void DrawTextPro(Font font, const char * text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint) { g_calls++; (void)font; (void)text; (void)position; (void)origin; (void)rotation; (void)fontSize; (void)spacing; (void)tint; }
void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint) { g_calls++; (void)font; (void)codepoint; (void)position; (void)fontSize; (void)tint; }
void SetTextLineSpacing(int spacing) { g_calls++; (void)spacing; }
int MeasureText(const char * text, int fontSize) { g_calls++; (void)text; (void)fontSize; int z; memset(&z, 0, sizeof z); return z; }
Vector2 MeasureTextEx(Font f, const char *t, float size, float sp) { g_calls++; Vector2 v = {(float)(int)strlen(t) * size + sp, (float)f.baseSize}; return v; }
int GetGlyphIndex(Font font, int codepoint) { g_calls++; (void)font; (void)codepoint; int z; memset(&z, 0, sizeof z); return z; }
unsigned int TextLength(const char * text) { g_calls++; (void)text; unsigned int z; memset(&z, 0, sizeof z); return z; }
bool TextIsEqual(const char * text1, const char * text2) { g_calls++; (void)text1; (void)text2; bool z; memset(&z, 0, sizeof z); return z; }
const char * TextSubtext(const char * text, int position, int length) { g_calls++; (void)text; (void)position; (void)length; return 0; }
const char * TextReplace(const char * text, const char * search, const char * replacement) { g_calls++; (void)text; (void)search; (void)replacement; return 0; }
int TextFindIndex(const char * text, const char * find) { g_calls++; (void)text; (void)find; int z; memset(&z, 0, sizeof z); return z; }
const char * TextToUpper(const char * text) { g_calls++; (void)text; return 0; }
const char * TextToLower(const char * text) { g_calls++; (void)text; return 0; }
const char * TextToPascal(const char * text) { g_calls++; (void)text; return 0; }
const char * TextToSnake(const char * text) { g_calls++; (void)text; return 0; }
const char * TextToCamel(const char * text) { g_calls++; (void)text; return 0; }
int TextToInteger(const char * text) { g_calls++; (void)text; int z; memset(&z, 0, sizeof z); return z; }
void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color) { g_calls++; (void)startPos; (void)endPos; (void)color; }
void DrawPoint3D(Vector3 position, Color color) { g_calls++; (void)position; (void)color; }
void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color) { g_calls++; (void)center; (void)radius; (void)rotationAxis; (void)rotationAngle; (void)color; }
void DrawTriangle3D(Vector3 v1, Vector3 v2, Vector3 v3, Color color) { g_calls++; (void)v1; (void)v2; (void)v3; (void)color; }
void DrawTriangleStrip3D(Vector3 * points, int pointCount, Color color) { g_calls++; (void)points; (void)pointCount; (void)color; }
void DrawCube(Vector3 position, float width, float height, float length, Color color) { g_calls++; (void)position; (void)width; (void)height; (void)length; (void)color; }
void DrawCubeV(Vector3 position, Vector3 size, Color color) { g_calls++; (void)position; (void)size; (void)color; }
void DrawCubeWires(Vector3 position, float width, float height, float length, Color color) { g_calls++; (void)position; (void)width; (void)height; (void)length; (void)color; }
void DrawCubeWiresV(Vector3 position, Vector3 size, Color color) { g_calls++; (void)position; (void)size; (void)color; }
void DrawSphere(Vector3 centerPos, float radius, Color color) { g_calls++; (void)centerPos; (void)radius; (void)color; }
void DrawSphereEx(Vector3 centerPos, float radius, int rings, int slices, Color color) { g_calls++; (void)centerPos; (void)radius; (void)rings; (void)slices; (void)color; }
void DrawSphereWires(Vector3 centerPos, float radius, int rings, int slices, Color color) { g_calls++; (void)centerPos; (void)radius; (void)rings; (void)slices; (void)color; }
void DrawCylinder(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color) { g_calls++; (void)position; (void)radiusTop; (void)radiusBottom; (void)height; (void)slices; (void)color; }
void DrawCylinderEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color) { g_calls++; (void)startPos; (void)endPos; (void)startRadius; (void)endRadius; (void)sides; (void)color; }
void DrawCylinderWires(Vector3 position, float radiusTop, float radiusBottom, float height, int slices, Color color) { g_calls++; (void)position; (void)radiusTop; (void)radiusBottom; (void)height; (void)slices; (void)color; }
void DrawCylinderWiresEx(Vector3 startPos, Vector3 endPos, float startRadius, float endRadius, int sides, Color color) { g_calls++; (void)startPos; (void)endPos; (void)startRadius; (void)endRadius; (void)sides; (void)color; }
void DrawCapsule(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color) { g_calls++; (void)startPos; (void)endPos; (void)radius; (void)slices; (void)rings; (void)color; }
void DrawCapsuleWires(Vector3 startPos, Vector3 endPos, float radius, int slices, int rings, Color color) { g_calls++; (void)startPos; (void)endPos; (void)radius; (void)slices; (void)rings; (void)color; }
void DrawPlane(Vector3 centerPos, Vector2 size, Color color) { g_calls++; (void)centerPos; (void)size; (void)color; }
void DrawRay(Ray ray, Color color) { g_calls++; (void)ray; (void)color; }
void DrawGrid(int slices, float spacing) { g_calls++; (void)slices; (void)spacing; }
Model LoadModel(const char * fileName) { g_calls++; (void)fileName; Model z; memset(&z, 0, sizeof z); return z; }
Model LoadModelFromMesh(Mesh mesh) { g_calls++; (void)mesh; Model z; memset(&z, 0, sizeof z); return z; }
bool IsModelValid(Model model) { g_calls++; (void)model; bool z; memset(&z, 0, sizeof z); return z; }
void UnloadModel(Model model) { g_calls++; (void)model; }
BoundingBox GetModelBoundingBox(Model m) { g_calls++; BoundingBox b = {{(float)m.meshCount, 0, 0}, {0, (float)m.boneCount, m.transform.m15}}; return b; }
void DrawModel(Model model, Vector3 position, float scale, Color tint) { g_calls++; (void)model; (void)position; (void)scale; (void)tint; }
void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) { g_calls++; (void)model; (void)position; (void)rotationAxis; (void)rotationAngle; (void)scale; (void)tint; }
void DrawModelWires(Model model, Vector3 position, float scale, Color tint) { g_calls++; (void)model; (void)position; (void)scale; (void)tint; }
void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint) { g_calls++; (void)model; (void)position; (void)rotationAxis; (void)rotationAngle; (void)scale; (void)tint; }
void DrawBoundingBox(BoundingBox box, Color color) { g_calls++; (void)box; (void)color; }
void DrawBillboard(Camera3D camera, Texture texture, Vector3 position, float scale, Color tint) { g_calls++; (void)camera; (void)texture; (void)position; (void)scale; (void)tint; }
void DrawBillboardRec(Camera3D camera, Texture texture, Rectangle source, Vector3 position, Vector2 size, Color tint) { g_calls++; (void)camera; (void)texture; (void)source; (void)position; (void)size; (void)tint; }
void UploadMesh(Mesh *mesh, bool dynamic) { g_calls++; (void)mesh; (void)dynamic; }
void UnloadMesh(Mesh mesh) { g_calls++; (void)mesh; }
void DrawMesh(Mesh mesh, Material material, Matrix transform) { g_calls++; (void)mesh; (void)material; (void)transform; }
BoundingBox GetMeshBoundingBox(Mesh mesh) { g_calls++; (void)mesh; BoundingBox z; memset(&z, 0, sizeof z); return z; }
void GenMeshTangents(Mesh *mesh) { g_calls++; (void)mesh; }
bool ExportMesh(Mesh mesh, const char * fileName) { g_calls++; (void)mesh; (void)fileName; bool z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshPoly(int sides, float radius) { g_calls++; (void)sides; (void)radius; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshPlane(float width, float length, int resX, int resZ) { g_calls++; (void)width; (void)length; (void)resX; (void)resZ; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshCube(float width, float height, float length) { g_calls++; (void)width; (void)height; (void)length; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshSphere(float radius, int rings, int slices) { g_calls++; (void)radius; (void)rings; (void)slices; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshHemiSphere(float radius, int rings, int slices) { g_calls++; (void)radius; (void)rings; (void)slices; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshCylinder(float radius, float height, int slices) { g_calls++; (void)radius; (void)height; (void)slices; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshCone(float radius, float height, int slices) { g_calls++; (void)radius; (void)height; (void)slices; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshTorus(float radius, float size, int radSeg, int sides) { g_calls++; (void)radius; (void)size; (void)radSeg; (void)sides; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshKnot(float radius, float size, int radSeg, int sides) { g_calls++; (void)radius; (void)size; (void)radSeg; (void)sides; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshHeightmap(Image heightmap, Vector3 size) { g_calls++; (void)heightmap; (void)size; Mesh z; memset(&z, 0, sizeof z); return z; }
Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize) { g_calls++; (void)cubicmap; (void)cubeSize; Mesh z; memset(&z, 0, sizeof z); return z; }
Material LoadMaterialDefault(void) { g_calls++; Material z; memset(&z, 0, sizeof z); return z; }
bool IsMaterialValid(Material material) { g_calls++; (void)material; bool z; memset(&z, 0, sizeof z); return z; }
void UnloadMaterial(Material material) { g_calls++; (void)material; }
void SetMaterialTexture(Material *material, int mapType, Texture texture) { g_calls++; (void)material; (void)mapType; (void)texture; }
void SetModelMeshMaterial(Model *model, int meshId, int materialId) { g_calls++; (void)model; (void)meshId; (void)materialId; }
ModelAnimation * LoadModelAnimations(const char * fileName, int *animCount) { g_calls++; (void)fileName; (void)animCount; return 0; }
void UpdateModelAnimation(Model m, ModelAnimation a, int frame) { g_calls++; g_last = m.meshCount + a.frameCount + a.boneCount + frame; }
void UnloadModelAnimation(ModelAnimation anim) { g_calls++; (void)anim; }
void UnloadModelAnimations(ModelAnimation * animations, int animCount) { g_calls++; (void)animations; (void)animCount; }
bool IsModelAnimationValid(Model model, ModelAnimation anim) { g_calls++; (void)model; (void)anim; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionSpheres(Vector3 center1, float radius1, Vector3 center2, float radius2) { g_calls++; (void)center1; (void)radius1; (void)center2; (void)radius2; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionBoxes(BoundingBox box1, BoundingBox box2) { g_calls++; (void)box1; (void)box2; bool z; memset(&z, 0, sizeof z); return z; }
bool CheckCollisionBoxSphere(BoundingBox box, Vector3 center, float radius) { g_calls++; (void)box; (void)center; (void)radius; bool z; memset(&z, 0, sizeof z); return z; }
RayCollision GetRayCollisionSphere(Ray r, Vector3 c, float rad) { g_calls++; RayCollision rc; memset(&rc, 0, sizeof rc); rc.hit = true; rc.distance = r.position.x + c.y + rad; rc.point = r.direction; rc.normal = c; return rc; }
RayCollision GetRayCollisionBox(Ray ray, BoundingBox box) { g_calls++; (void)ray; (void)box; RayCollision z; memset(&z, 0, sizeof z); return z; }
RayCollision GetRayCollisionMesh(Ray ray, Mesh mesh, Matrix transform) { g_calls++; (void)ray; (void)mesh; (void)transform; RayCollision z; memset(&z, 0, sizeof z); return z; }
RayCollision GetRayCollisionTriangle(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3) { g_calls++; (void)ray; (void)p1; (void)p2; (void)p3; RayCollision z; memset(&z, 0, sizeof z); return z; }
RayCollision GetRayCollisionQuad(Ray ray, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4) { g_calls++; (void)ray; (void)p1; (void)p2; (void)p3; (void)p4; RayCollision z; memset(&z, 0, sizeof z); return z; }
void InitAudioDevice(void) { g_calls++; }
void CloseAudioDevice(void) { g_calls++; }
bool IsAudioDeviceReady(void) { g_calls++; bool z; memset(&z, 0, sizeof z); return z; }
void SetMasterVolume(float volume) { g_calls++; (void)volume; }
float GetMasterVolume(void) { g_calls++; float z; memset(&z, 0, sizeof z); return z; }
Wave LoadWave(const char * fileName) { g_calls++; (void)fileName; Wave z; memset(&z, 0, sizeof z); return z; }
bool IsWaveValid(Wave wave) { g_calls++; (void)wave; bool z; memset(&z, 0, sizeof z); return z; }
Sound LoadSound(const char * fileName) { g_calls++; (void)fileName; Sound z; memset(&z, 0, sizeof z); return z; }
Sound LoadSoundFromWave(Wave wave) { g_calls++; (void)wave; Sound z; memset(&z, 0, sizeof z); return z; }
Sound LoadSoundAlias(Sound source) { g_calls++; (void)source; Sound z; memset(&z, 0, sizeof z); return z; }
bool IsSoundValid(Sound sound) { g_calls++; (void)sound; bool z; memset(&z, 0, sizeof z); return z; }
void UpdateSound(Sound sound, void * data, int sampleCount) { g_calls++; (void)sound; (void)data; (void)sampleCount; }
void UnloadWave(Wave wave) { g_calls++; (void)wave; }
void UnloadSound(Sound sound) { g_calls++; (void)sound; }
void UnloadSoundAlias(Sound alias) { g_calls++; (void)alias; }
bool ExportWave(Wave wave, const char * fileName) { g_calls++; (void)wave; (void)fileName; bool z; memset(&z, 0, sizeof z); return z; }
void PlaySound(Sound sound) { g_calls++; (void)sound; }
void StopSound(Sound sound) { g_calls++; (void)sound; }
void PauseSound(Sound sound) { g_calls++; (void)sound; }
void ResumeSound(Sound sound) { g_calls++; (void)sound; }
bool IsSoundPlaying(Sound sound) { g_calls++; (void)sound; bool z; memset(&z, 0, sizeof z); return z; }
void SetSoundVolume(Sound sound, float volume) { g_calls++; (void)sound; (void)volume; }
void SetSoundPitch(Sound sound, float pitch) { g_calls++; (void)sound; (void)pitch; }
void SetSoundPan(Sound sound, float pan) { g_calls++; (void)sound; (void)pan; }
Wave WaveCopy(Wave wave) { g_calls++; (void)wave; Wave z; memset(&z, 0, sizeof z); return z; }
void WaveCrop(Wave *wave, int initFrame, int finalFrame) { g_calls++; (void)wave; (void)initFrame; (void)finalFrame; }
void WaveFormat(Wave *wave, int sampleRate, int sampleSize, int channels) { g_calls++; (void)wave; (void)sampleRate; (void)sampleSize; (void)channels; }
float * LoadWaveSamples(Wave wave) { g_calls++; (void)wave; return 0; }
void UnloadWaveSamples(float * samples) { g_calls++; (void)samples; }
Music LoadMusicStream(const char * fileName) { g_calls++; (void)fileName; Music z; memset(&z, 0, sizeof z); return z; }
bool IsMusicValid(Music music) { g_calls++; (void)music; bool z; memset(&z, 0, sizeof z); return z; }
void UnloadMusicStream(Music music) { g_calls++; (void)music; }
void PlayMusicStream(Music music) { g_calls++; (void)music; }
bool IsMusicStreamPlaying(Music music) { g_calls++; (void)music; bool z; memset(&z, 0, sizeof z); return z; }
void UpdateMusicStream(Music music) { g_calls++; (void)music; }
void StopMusicStream(Music music) { g_calls++; (void)music; }
void PauseMusicStream(Music music) { g_calls++; (void)music; }
void ResumeMusicStream(Music music) { g_calls++; (void)music; }
void SeekMusicStream(Music music, float position) { g_calls++; (void)music; (void)position; }
void SetMusicVolume(Music music, float volume) { g_calls++; (void)music; (void)volume; }
void SetMusicPitch(Music music, float pitch) { g_calls++; (void)music; (void)pitch; }
void SetMusicPan(Music music, float pan) { g_calls++; (void)music; (void)pan; }
float GetMusicTimeLength(Music music) { g_calls++; (void)music; float z; memset(&z, 0, sizeof z); return z; }
float GetMusicTimePlayed(Music music) { g_calls++; (void)music; float z; memset(&z, 0, sizeof z); return z; }
