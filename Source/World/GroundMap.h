#pragma once
#include <cstdint>
#include <optional>

#include <JugX/Result.h>
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

namespace slug
{

// ===========================================
//  GroundBoundsPolicy
//   맵 밖 픽셀을 고체로 볼지 여부. 기본: 아래쪽만 고체(맵 바닥 아래로 떨어지지 않음).
// ===========================================

struct GroundBoundsPolicy
{
    bool bSolidLeft   = false;
    bool bSolidRight  = false;
    bool bSolidTop    = false;
    bool bSolidBottom = true;
};

// ===========================================
//  GroundMap
//   픽셀 단위 충돌맵 (1비트/픽셀). 고체 = 비트 1.
//   저장 형식은 Baker 의 Ground/*.bin 과 동일: 행 우선, 행은 바이트 경계 정렬, LSB 가 왼쪽 픽셀.
//   조회는 비트 연산만 사용한다 (가상 호출 없음).
//
//   좌표 규약: 픽셀 (x, y) 는 연속 좌표 [x, x+1) x [y, y+1) 를 덮는다.
//   "지면 높이 y" 는 고체 픽셀 행의 윗변 좌표이며, 발이 y 에 있으면 그 행 위에 서 있는 것이다.
// ===========================================

class GroundMap
{
public:
    static constexpr uint8_t kSolidMask = 1;

    // ===========================================
    //  생성
    // ===========================================

    [[nodiscard]] static jug::Result<GroundMap> Load(const jug::FilePath& _path);
    [[nodiscard]] static jug::Result<GroundMap> LoadFromMemory(jug::Span<const uint8_t> _fileData);

    // 헤더 없이 비트 배열만으로 만든다 (파일 포맷과 동일한 배치). 테스트/절차 생성용.
    [[nodiscard]] static GroundMap MakeFromBits(int _width, int _height, jug::Span<const uint8_t> _packedBits);

    // 모두 공기인 맵
    [[nodiscard]] static GroundMap MakeEmpty(int _width, int _height);

    // 문자 한 개가 한 픽셀인 ASCII 그림으로 만든다. _solid 문자가 고체. 행 길이가 달라도 최대 폭 기준 (부족분은 공기).
    [[nodiscard]] static GroundMap MakeFromRows(jug::Span<const jug::StringView> _rows, char _solid = '#');

    GroundMap() = default;

    // ===========================================
    //  편집 (테스트/동적 지형용)
    // ===========================================

    void SetSolid(int _x, int _y, bool _bSolid);
    void FillRect(int _x0, int _y0, int _x1, int _y1, bool _bSolid);   // 닫힌 구간 [x0,x1] x [y0,y1], 맵 밖은 무시

    // ===========================================
    //  정보
    // ===========================================

    [[nodiscard]] int      GetWidth() const { return m_width; }
    [[nodiscard]] int      GetHeight() const { return m_height; }
    [[nodiscard]] bool     IsEmpty() const { return m_width == 0 || m_height == 0; }
    [[nodiscard]] uint64_t CountSolidPixels() const;

    void                                    SetBoundsPolicy(const GroundBoundsPolicy& _policy) { m_policy = _policy; }
    [[nodiscard]] const GroundBoundsPolicy& GetBoundsPolicy() const { return m_policy; }

    // ===========================================
    //  질의
    // ===========================================

    [[nodiscard]] bool IsSolid(
        const int _x,
        const int _y) const
    {
        if (static_cast<unsigned>(_x) >= static_cast<unsigned>(m_width) || static_cast<unsigned>(_y) >= static_cast<unsigned>(m_height))
        {
            return IsOutOfBoundsSolid_(_x, _y);
        }
        return ((m_bits[static_cast<size_t>(_y) * m_stride + static_cast<size_t>(_x >> 3)] >> (_x & 7)) & kSolidMask) != 0;
    }

    // 연속 좌표의 점이 속한 픽셀 조회
    [[nodiscard]] bool IsSolidAt(jug::VECTOR2 _point) const;

    // 닫힌 정수 구간 [x0,x1] x [y0,y1] 안에 고체 픽셀이 하나라도 있는가 (맵 밖 정책 포함)
    [[nodiscard]] bool IsRectSolid(int _x0, int _y0, int _x1, int _y1) const;

    // 열 _x 에서 _fromY 부터 아래로 [fromY, fromY + maxDist] 범위의 첫 고체 행. 없으면 nullopt.
    [[nodiscard]] std::optional<int> FindGroundY(int _x, int _fromY, int _maxDist) const;

    // 열 _x 에서 _fromY 부터 위로 [fromY - maxDist, fromY] 범위의 첫 고체 행(천장). 없으면 nullopt.
    [[nodiscard]] std::optional<int> FindCeilingY(int _x, int _fromY, int _maxDist) const;

    // (_x, _y) 가 고체 속에 파묻혀 있을 때, 위로 최대 _maxStep 칸 올라가 만나는 지면(연속된 고체 구간의 맨 윗 행).
    // (_x, _y) 가 고체가 아니거나 _maxStep 안에 벗어나지 못하면 nullopt.
    [[nodiscard]] std::optional<int> FindSurfaceUp(int _x, int _y, int _maxStep) const;

    // 지면의 기울기 dy/dx (+y 아래, 오른쪽으로 내려가면 양수). _y 근처 ±_searchRange 에서 x±_halfWidth 의 지면 높이를 비교한다.
    // 한쪽이라도 지면을 찾지 못하면 0.
    [[nodiscard]] float EstimateSlope(int _x, int _y, int _halfWidth, int _searchRange) const;

    // 위쪽을 향하는 근사 법선 (정규화됨). 평지면 (0, -1).
    [[nodiscard]] jug::VECTOR2 EstimateNormal(int _x, int _y, int _halfWidth, int _searchRange) const;

private:
    [[nodiscard]] bool IsOutOfBoundsSolid_(int _x, int _y) const;

    int                  m_width  = 0;
    int                  m_height = 0;
    size_t               m_stride = 0;   // 행당 바이트 수
    jug::Vector<uint8_t> m_bits   = {};
    GroundBoundsPolicy   m_policy = {};
};

}   // namespace slug
