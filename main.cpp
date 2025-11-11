#include <Novice.h>
#include <cmath>

const char kWindowTitle[] = "L";

// Quaternion構造体
struct Quaternion {
    float x;
    float y;
    float z;
    float w;
};

// Quaternionの積
Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs) {
    Quaternion result;
    result.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
    result.y = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x;
    result.z = lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w;
    result.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
    return result;
}

// 単位Quaternionを返す
Quaternion IdentityQuaternion() {
    return { 0.0f, 0.0f, 0.0f, 1.0f };
}

// 共役Quaternionを返す
Quaternion Conjugate(const Quaternion& quaternion) {
    return { -quaternion.x, -quaternion.y, -quaternion.z, quaternion.w };
}

// Quaternionのnormを返す
float Norm(const Quaternion& quaternion) {
    return std::sqrt(
        quaternion.x * quaternion.x +
        quaternion.y * quaternion.y +
        quaternion.z * quaternion.z +
        quaternion.w * quaternion.w
    );
}

// 正規化したQuaternionを返す
Quaternion Normalize(const Quaternion& quaternion) {
    float norm = Norm(quaternion);
    if (norm == 0.0f) return { 0.0f, 0.0f, 0.0f, 0.0f };
    return {
        quaternion.x / norm,
        quaternion.y / norm,
        quaternion.z / norm,
        quaternion.w / norm
    };
}

// 逆Quaternionを返す
Quaternion Inverse(const Quaternion& quaternion) {
    Quaternion conj = Conjugate(quaternion);
    float normSq = quaternion.x * quaternion.x +
        quaternion.y * quaternion.y +
        quaternion.z * quaternion.z +
        quaternion.w * quaternion.w;

    if (normSq == 0.0f) return { 0.0f, 0.0f, 0.0f, 0.0f };

    return {
        conj.x / normSq,
        conj.y / normSq,
        conj.z / normSq,
        conj.w / normSq
    };
}

// -----------------------------
// クォータニオンを整列表示する関数
// -----------------------------
void PrintQuaternion(int x, int& y, const Quaternion& q, const char* label) {
    Novice::ScreenPrintf(x, y, "%.2f   %.2f   %.2f   %.2f   : %s",
        q.x, q.y, q.z, q.w, label);
    y += 20;
}

// Norm用（1行出力）
void PrintNorm(int x, int& y, float norm, const char* label) {
    Novice::ScreenPrintf(x, y, "%.2f                            : %s", norm, label);
    y += 20;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

    Novice::Initialize(kWindowTitle, 1280, 720);

    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    Quaternion q1 = { 2.0f, 3.0f, 4.0f, 1.0f };
    Quaternion q2 = { 1.0f, 3.0f, 5.0f, 2.0f };

    Quaternion identity = IdentityQuaternion();
    Quaternion conj = Conjugate(q1);
    Quaternion inv = Inverse(q1);
    Quaternion normal = Normalize(q1);
    Quaternion mul1 = Multiply(q1, q2);
    Quaternion mul2 = Multiply(q2, q1);
    float norm = Norm(q1);

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        ///
        /// ↓描画処理ここから
        ///

        int x = 20;
        int y = 20;

        PrintQuaternion(x, y, identity, "Identity");
        PrintQuaternion(x, y, conj, "Conjugate");
        PrintQuaternion(x, y, inv, "Inverse");
        PrintQuaternion(x, y, normal, "Normalize");
        PrintQuaternion(x, y, mul1, "Multiply(q1, q2)");
        PrintQuaternion(x, y, mul2, "Multiply(q2, q1)");
        PrintNorm(x, y, norm, "Norm");

        ///
        /// ↑描画処理ここまで
        ///

        Novice::EndFrame();

        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }

    Novice::Finalize();
    return 0;
}