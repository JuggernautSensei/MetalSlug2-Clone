#include <cstdio>
#include <cstdlib>

#include "Base/Result.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Tools/Baker/Baker.h"
#include "Tools/Baker/BakerUtil.h"

namespace
{

using namespace slug::baker;
using slug::ARRAY;
using slug::String;
using slug::StringView;

void PrintUsage_()
{
    std::puts(
        "SlugBaker - legacy resources -> Assets/Baked\n"
        "\n"
        "usage: SlugBaker --legacy <Resources dir> [--out <dir>] [options]\n"
        "\n"
        "  --legacy <dir>          legacy Resources directory (contains ResourceData/)\n"
        "  --out <dir>             output root (default: <Assets>/Baked, Assets found from the exe location)\n"
        "  --only <a,b,..>         partial bake: player enemy effect npc sheet sprites ground sounds\n"
        "  --sheets <sheets.json>  raw sprite sheet clips (default: <Assets>/Source/sheets.json when it exists)\n"
        "  --no-sheets             skip the sheet pass\n"
        "  --ground-edits <json>   ground pixel edits (default: <out>/../Source/GroundEdits.json when it exists)\n"
        "  --sheet-preview <png>   debug: write Captures/SheetPreview/<name>.png with numbered rects (no legacy bake)\n"
        "  --sheet-preview-scale <n>  preview magnification, nearest neighbour (default 1)\n"
        "  --threshold <B,RG,A>    ground solid thresholds: B>=B, R<=RG, G<=RG, A>=A (default 192,96,128)\n"
        "  --ground-blue-min <n>   ground: minimum blue (default 192)\n"
        "  --ground-rg-max <n>     ground: maximum red/green (default 96)\n"
        "  --ground-alpha-min <n>  ground: minimum alpha (default 128)\n"
        "  --page-size <n>         atlas page size (default 2048)\n"
        "  --padding <n>           atlas gap in pixels (default 1)\n"
        "  --standalone-size <n>   cells larger than this on a side become standalone PNGs (default 1024)\n"
        "  --alpha-threshold <n>   trim treats alpha > n as content (default 0)\n"
        "  --no-alpha-magenta-key  keep opaque (255,0,255) pixels in UseTransparent=2 textures (default: key them out)\n"
        "  --threads <n>           worker threads (default: auto)\n"
        "  --help                  show this help");
}

[[nodiscard]] bool ParseInt_(
    const String& _text,
    int&          _out)
{
    return ParseIntStrict(_text, _out);
}

// 명령행 인자를 하나씩 소비하는 커서. 옵션 뒤의 값을 읽을 때 오류 메시지까지 처리한다.
class ArgReader
{
public:
    ArgReader(
        const int _argc,
        char**    _argv)
        : m_argc(_argc)
        , m_pArgv(_argv)
    {
    }

    // 다음 인자로 넘어간다. 더 없으면 false.
    [[nodiscard]] bool Advance()
    {
        ++m_index;
        if (m_index >= m_argc)
        {
            return false;
        }
        m_arg = m_pArgv[m_index];
        return true;
    }

    [[nodiscard]] const String& GetArg() const
    {
        return m_arg;
    }

    [[nodiscard]] bool NextValue(
        String& _outValue)
    {
        if (m_index + 1 >= m_argc)
        {
            Log("ERROR: {} needs a value", m_arg);
            return false;
        }
        ++m_index;
        _outValue = m_pArgv[m_index];
        return true;
    }

    [[nodiscard]] bool NextInt(
        int& _outValue)
    {
        String text;
        if (!NextValue(text))
        {
            return false;
        }
        if (!ParseInt_(text, _outValue))
        {
            Log("ERROR: {} expects an integer, got '{}'", m_arg, text);
            return false;
        }
        return true;
    }

private:
    int    m_argc  = 0;
    char** m_pArgv = nullptr;
    int    m_index = 0;
    String m_arg   = {};
};

}   // namespace

int main(
    int    _argc,
    char** _argv)
{
#ifdef SLUG_ASSET_DIR
    slug::SetAssetRootFallback(SLUG_ASSET_DIR);
#endif
    BakerOptions options;
    ArgReader    reader { _argc, _argv };

    while (reader.Advance())
    {
        const String& arg = reader.GetArg();
        String        value;
        if (arg == "--help" || arg == "-h")
        {
            PrintUsage_();
            return 0;
        }
        else if (arg == "--legacy")
        {
            if (!reader.NextValue(value))
            {
                return 2;
            }
            options.legacyRoot = value;
        }
        else if (arg == "--out")
        {
            if (!reader.NextValue(value))
            {
                return 2;
            }
            options.outRoot = value;
        }
        else if (arg == "--only")
        {
            if (!reader.NextValue(value))
            {
                return 2;
            }
            size_t pos = 0;
            while (pos <= value.size())
            {
                const size_t comma = value.find(',', pos);
                const String token(TrimView(StringView(value).substr(pos, comma == String::npos ? String::npos : comma - pos)));
                if (!token.empty())
                {
                    options.only.insert(token);
                }
                if (comma == String::npos)
                {
                    break;
                }
                pos = comma + 1;
            }
        }
        else if (arg == "--threshold")
        {
            if (!reader.NextValue(value))
            {
                return 2;
            }
            ARRAY<int, 3> parts = {};
            size_t        count = 0;
            size_t        pos   = 0;
            while (count < parts.size() && pos <= value.size())
            {
                const size_t comma = value.find(',', pos);
                if (!ParseInt_(value.substr(pos, comma == String::npos ? String::npos : comma - pos), parts[count]))
                {
                    break;
                }
                ++count;
                if (comma == String::npos)
                {
                    break;
                }
                pos = comma + 1;
            }
            if (count != parts.size())
            {
                Log("ERROR: --threshold expects B,RG,A (e.g. 192,96,128)");
                return 2;
            }
            options.ground.blueMin  = parts[0];
            options.ground.redMax   = parts[1];
            options.ground.greenMax = parts[1];
            options.ground.alphaMin = parts[2];
        }
        else if (arg == "--ground-blue-min")
        {
            if (!reader.NextInt(options.ground.blueMin))
            {
                return 2;
            }
        }
        else if (arg == "--ground-rg-max")
        {
            if (!reader.NextInt(options.ground.redMax))
            {
                return 2;
            }
            options.ground.greenMax = options.ground.redMax;
        }
        else if (arg == "--ground-alpha-min")
        {
            if (!reader.NextInt(options.ground.alphaMin))
            {
                return 2;
            }
        }
        else if (arg == "--page-size")
        {
            if (!reader.NextInt(options.pageSize))
            {
                return 2;
            }
        }
        else if (arg == "--padding")
        {
            if (!reader.NextInt(options.padding))
            {
                return 2;
            }
        }
        else if (arg == "--standalone-size")
        {
            if (!reader.NextInt(options.standaloneSize))
            {
                return 2;
            }
        }
        else if (arg == "--alpha-threshold")
        {
            if (!reader.NextInt(options.alphaThreshold))
            {
                return 2;
            }
        }
        else if (arg == "--no-alpha-magenta-key")
        {
            options.bKeyMagentaInAlpha = false;
        }
        else if (arg == "--sheets")
        {
            if (!reader.NextValue(value))
            {
                return 2;
            }
            options.sheetsPath = value;
        }
        else if (arg == "--no-sheets")
        {
            options.bNoSheets = true;
        }
        else if (arg == "--ground-edits")
        {
            if (!reader.NextValue(value))
            {
                return 2;
            }
            options.groundEditsPath = value;
        }
        else if (arg == "--sheet-preview")
        {
            if (!reader.NextValue(value))
            {
                return 2;
            }
            options.sheetPreview = value;
        }
        else if (arg == "--sheet-preview-scale")
        {
            if (!reader.NextInt(options.sheetPreviewScale))
            {
                return 2;
            }
            if (options.sheetPreviewScale < 1)
            {
                options.sheetPreviewScale = 1;
            }
        }
        else if (arg == "--threads")
        {
            if (!reader.NextInt(options.threads))
            {
                return 2;
            }
        }
        else
        {
            Log("ERROR: unknown argument '{}'", arg);
            PrintUsage_();
            return 2;
        }
    }

    if (options.legacyRoot.empty() && options.sheetPreview.empty())
    {
        Log("ERROR: --legacy <Resources dir> is required");
        PrintUsage_();
        return 2;
    }
    if (options.pageSize < 64 || options.padding < 0)
    {
        Log("ERROR: invalid --page-size / --padding");
        return 2;
    }

    Log("SlugBaker: legacy={} out={}", options.legacyRoot.string(), options.outRoot.string());
    if (const slug::Result<void> result = RunBaker(options); !result)
    {
        Log("FAILED: {}", result.GetError().MakeMessage());
        return 1;
    }
    return 0;
}
