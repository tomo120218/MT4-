#include <Novice.h>
#include <cmath>
#include <cstring>

const char kWindowTitle[] = "MT";
static const int kRowHeight = 20; // 1行の高さ

// ===== 最小限の型 =====
struct Vector3 { float x, y, z; };
struct Matrix4x4 { float m[4][4]; };

// ===== ユーティリティ =====
static inline float Dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
static inline Vector3 Cross(const Vector3& a, const Vector3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}
static inline float Length(const Vector3& v) {
    return std::sqrt(Dot(v, v));
}
static inline Vector3 Normalize(const Vector3& v) {
    float len = Length(v);
    if (len <= 0.0f) return { 0.0f, 0.0f, 0.0f };
    float inv = 1.0f / len;
    return { v.x * inv, v.y * inv, v.z * inv };
}
static inline float Clamp(float x, float a, float b) {
    return (x < a) ? a : (x > b ? b : x);
}

static inline Matrix4x4 Identity4x4() {
    Matrix4x4 A{};
    for (int i = 0; i < 4; ++i) A.m[i][i] = 1.0f;
    return A;
}

// ===== 任意軸回転行列：Rodriguesの回転公式 =====
Matrix4x4 MakeRotateAxisAngle(const Vector3& axisRaw, float angle) {
    const Vector3 a = Normalize(axisRaw);
    const float x = a.x, y = a.y, z = a.z;
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const float t = 1.0f - c;

    Matrix4x4 R = Identity4x4();

    R.m[0][0] = t * x * x + c;     R.m[0][1] = t * x * y + s * z; R.m[0][2] = t * x * z - s * y; R.m[0][3] = 0.0f;
    R.m[1][0] = t * y * x - s * z; R.m[1][1] = t * y * y + c;     R.m[1][2] = t * y * z + s * x; R.m[1][3] = 0.0f;
    R.m[2][0] = t * z * x + s * y; R.m[2][1] = t * z * y - s * x; R.m[2][2] = t * z * z + c;     R.m[2][3] = 0.0f;
    R.m[3][0] = 0.0f;              R.m[3][1] = 0.0f;              R.m[3][2] = 0.0f;              R.m[3][3] = 1.0f;

    return R;
}

// ===== ある方向 from を ある方向 to へ向ける回転行列 =====
Matrix4x4 DirectionToDirection(const Vector3& fromRaw, const Vector3& toRaw) {
    const float EPS = 1e-6f;
    const float PI = 3.14159265358979323846f;

    // 単位ベクトル化
    Vector3 from = Normalize(fromRaw);
    Vector3 to = Normalize(toRaw);

    // 角度の余弦（内積）を計算しクランプ
    float d = Clamp(Dot(from, to), -1.0f, 1.0f);

    // 同方向（角度 ≈ 0）→ 単位行列
    if (std::fabs(d - 1.0f) <= 1e-6f) {
        return Identity4x4();
    }

    // 正反対（角度 ≈ π）→ from と直交する“決め打ち軸”で 180 度回転
    if (std::fabs(d + 1.0f) <= 1e-6f) {

        // 参照軸を固定（Z を優先、Z とほぼ平行なら Y）
        Vector3 ref = (std::fabs(from.z) < 0.999f) ? Vector3{ 0.0f, 0.0f, 1.0f }
        : Vector3{ 0.0f, 1.0f, 0.0f };

        // from と直交する軸（順序は cross(ref, from) で統一）
        Vector3 axis = Normalize(Cross(ref, from));

        return MakeRotateAxisAngle(axis, PI);
    }


    // 一般ケース：軸 = from × to、角度 = acos(dot)
    Vector3 axis = Cross(from, to);
    if (Length(axis) <= EPS) {
        // 数値的に軸が取りにくいほどほぼ平行 → 回転なしで妥協
        return Identity4x4();
    }
    float angle = std::acos(d);
    return MakeRotateAxisAngle(axis, angle);
}

// 画面に4x4行列を表示（小数3桁）— 整形なし：-0.000 もそのまま出る
void MatrixScreenPrintf(int x, int y, const Matrix4x4& M, const char* title) {
    Novice::ScreenPrintf(x, y + kRowHeight * 0, "%s", title);
    Novice::ScreenPrintf(x, y + kRowHeight * 1, "%.3f %.3f %.3f %.3f",
        M.m[0][0], M.m[0][1], M.m[0][2], M.m[0][3]);
    Novice::ScreenPrintf(x, y + kRowHeight * 2, "%.3f %.3f %.3f %.3f",
        M.m[1][0], M.m[1][1], M.m[1][2], M.m[1][3]);
    Novice::ScreenPrintf(x, y + kRowHeight * 3, "%.3f %.3f %.3f %.3f",
        M.m[2][0], M.m[2][1], M.m[2][2], M.m[2][3]);
    Novice::ScreenPrintf(x, y + kRowHeight * 4, "%.3f %.3f %.3f %.3f",
        M.m[3][0], M.m[3][1], M.m[3][2], M.m[3][3]);
}



// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // ライブラリの初期化
    Novice::Initialize(kWindowTitle, 1280, 720);

    // キー入力結果を受け取る箱
    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    // ===== テストケースの定義 =====
    Vector3 from0 = Normalize(Vector3{ 1.0f, 0.7f, 0.5f });
    Vector3 to0 = { -from0.x, -from0.y, -from0.z };          // 反転（正反対）
    Vector3 from1 = Normalize(Vector3{ -0.6f, 0.9f, 0.2f });
    Vector3 to1 = Normalize(Vector3{ 0.4f, 0.7f,-0.5f });

    // +X → -X（正反対）・ from0→to0（正反対）・ from1→to1（一般）
    Matrix4x4 rotateMatrix0 = DirectionToDirection(
        Normalize(Vector3{ 1.0f, 0.0f, 0.0f }),
        Normalize(Vector3{ -1.0f, 0.0f, 0.0f })
    );
    Matrix4x4 rotateMatrix1 = DirectionToDirection(from0, to0);
    Matrix4x4 rotateMatrix2 = DirectionToDirection(from1, to1);

    // ウィンドウの×ボタンが押されるまでループ
    while (Novice::ProcessMessage() == 0) {
        // フレームの開始
        Novice::BeginFrame();

        // キー入力を受け取る
        std::memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        /// 
        /// ↓描画処理ここから 
        /// 

        /// 
        /// ↑描画処理ここまで 
        /// 

        /// 
        /// ↓描画処理ここから
        /// 

        MatrixScreenPrintf(0, 0, rotateMatrix0, "rotateMatrix0");
        MatrixScreenPrintf(0, kRowHeight * 5, rotateMatrix1, "rotateMatrix1");
        MatrixScreenPrintf(0, kRowHeight * 10, rotateMatrix2, "rotateMatrix2");

        /// 
        /// ↑描画処理ここまで 
        /// 

        // フレームの終了
        Novice::EndFrame();

        // ESCキーが押されたらループを抜ける
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }

    // ライブラリの終了
    Novice::Finalize();
    return 0;
}