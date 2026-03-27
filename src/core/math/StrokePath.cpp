#include "core/math/StrokePath.h"

#include <algorithm>

void StrokePath::clear()
{
    m_commands.clear();
}

void StrokePath::moveTo(const Vec2& p)
{
    m_commands.push_back({CmdType::MoveTo, p, {}, {}});
}

void StrokePath::lineTo(const Vec2& p)
{
    m_commands.push_back({CmdType::LineTo, p, {}, {}});
}

void StrokePath::quadTo(const Vec2& c, const Vec2& p)
{
    m_commands.push_back({CmdType::QuadTo, c, p, {}});
}

void StrokePath::cubicTo(const Vec2& c1, const Vec2& c2, const Vec2& p)
{
    m_commands.push_back({CmdType::CubicTo, c1, c2, p});
}

void StrokePath::closePath()
{
    m_commands.push_back({CmdType::ClosePath, {}, {}, {}});
}

std::vector<Vec2> StrokePath::flatten(int samples_per_curve) const
{
    const int samples = std::max(samples_per_curve, 2);
    std::vector<Vec2> out;
    Vec2 current{};
    Vec2 subpath_start{};
    bool has_current = false;

    for (const Command& cmd : m_commands) {
        switch (cmd.type) {
            case CmdType::MoveTo:
                current = cmd.a;
                subpath_start = cmd.a;
                has_current = true;
                out.push_back(current);
                break;

            case CmdType::LineTo:
                if (!has_current) break;
                current = cmd.a;
                out.push_back(current);
                break;

            case CmdType::QuadTo:
                if (!has_current) break;
                for (int i = 1; i <= samples; ++i) {
                    const double t = static_cast<double>(i) / static_cast<double>(samples);
                    out.push_back(BezierQuadratic{current, cmd.a, cmd.b}.eval(t));
                }
                current = cmd.b;
                break;

            case CmdType::CubicTo:
                if (!has_current) break;
                for (int i = 1; i <= samples; ++i) {
                    const double t = static_cast<double>(i) / static_cast<double>(samples);
                    out.push_back(BezierCubic{current, cmd.a, cmd.b, cmd.c}.eval(t));
                }
                current = cmd.c;
                break;

            case CmdType::ClosePath:
                if (!has_current) break;
                if ((current - subpath_start).length() > 1.0e-9)
                    out.push_back(subpath_start);
                current = subpath_start;
                break;
        }
    }

    return out;
}

std::vector<Vec2> StrokePath::controlPolygon() const
{
    std::vector<Vec2> out;
    for (const Command& cmd : m_commands) {
        switch (cmd.type) {
            case CmdType::MoveTo:
            case CmdType::LineTo:
                out.push_back(cmd.a);
                break;
            case CmdType::QuadTo:
                out.push_back(cmd.a);
                out.push_back(cmd.b);
                break;
            case CmdType::CubicTo:
                out.push_back(cmd.a);
                out.push_back(cmd.b);
                out.push_back(cmd.c);
                break;
            case CmdType::ClosePath:
                break;
        }
    }
    return out;
}
