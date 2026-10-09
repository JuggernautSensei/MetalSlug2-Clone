#include <cstdio>
#include "Base/Result.h"
#include "Core/AssetPath.h"
#include <cstdlib>
#include <string>

#include "Tools/Baker/Baker.h"
#include "Tools/Baker/BakerUtil.h"

namespace
{

using namespace slug::baker;

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
    const std::string& _text,
    int&               _out)
{
    return ParseIntStrict(_text, _out);
}
}

int main(
    int    _argc,
    char** _argv)
{
#ifdef SLUG_ASSET_DIR
    slug::SetAssetRootFallback(SLUG_ASSET_DIR);
#endif
    BakerOptions options;

    for (int i = 1; i < _argc; ++i)
    {
        const std::string arg = _argv[i];
        auto              next = [&](std::string& _value) -> bool
        {
            if (i + 1 >= _argc)
            {
                Log("ERROR: {} needs a value", arg);
                return false;
            }
            _value = _argv[++i];
            return true;
        };
        auto nextInt = [&](int& _value) -> bool
        {
            std::string text;
            if (!next(text))
            {
                return false;
            }
            if (!ParseInt_(text, _value))
            {
                Log("ERROR: {} expects an integer, got '{}'", arg, text);
                return false;
            }
            return true;
        };

        std::string value;
        if (arg == "--help" || arg == "-h")
        {
            PrintUsage_();
            return 0;
        }
        else if (arg == "--legacy")
        {
            if (!next(value)) { return 2; }
            options.legacyRoot = value;
        }
        else if (arg == "--out")
        {
            if (!next(value)) { return 2; }
            options.outRoot = value;
        }
        else if (arg == "--only")
        {
            if (!next(value)) { return 2; }
            size_t pos = 0;
            while (pos <= value.size())
            {
                const size_t comma = value.find(',', pos);
                const std::string token(TrimView(std::string_view(value).substr(pos, comma == std::string::npos ? std::string::npos : comma - pos)));
                if (!token.empty())
                {
                    options.only.insert(token);
                }
                if (comma == std::string::npos) { break; }
                pos = comma + 1;
            }
        }
        else if (arg == "--threshold")
        {
            if (!next(value)) { return 2; }
            int parts[3] = {};
            int count    = 0;
            size_t pos   = 0;
            while (count < 3 && pos <= value.size())
            {
                const size_t comma = value.find(',', pos);
                if (!ParseInt_(value.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos), parts[count]))
                {
                    break;
                }
                ++count;
                if (comma == std::string::npos) { break; }
                pos = comma + 1;
            }
            if (count != 3)
            {
                Log("ERROR: --threshold expects B,RG,A (e.g. 192,96,128)");
                return 2;
            }
            options.ground.blueMin  = parts[0];
            options.ground.redMax   = parts[1];
            options.ground.greenMax = parts[1];
            options.ground.alphaMin = parts[2];
        }
        else if (arg == "--ground-blue-min") { if (!nextInt(options.ground.blueMin)) { return 2; } }
        else if (arg == "--ground-rg-max")
        {
            if (!nextInt(options.ground.redMax)) { return 2; }
            options.ground.greenMax = options.ground.redMax;
        }
        else if (arg == "--ground-alpha-min") { if (!nextInt(options.ground.alphaMin)) { return 2; } }
        else if (arg == "--page-size") { if (!nextInt(options.pageSize)) { return 2; } }
        else if (arg == "--padding") { if (!nextInt(options.padding)) { return 2; } }
        else if (arg == "--standalone-size") { if (!nextInt(options.standaloneSize)) { return 2; } }
        else if (arg == "--alpha-threshold") { if (!nextInt(options.alphaThreshold)) { return 2; } }
        else if (arg == "--no-alpha-magenta-key") { options.bKeyMagentaInAlpha = false; }
        else if (arg == "--sheets")
        {
            if (!next(value)) { return 2; }
            options.sheetsPath = value;
        }
        else if (arg == "--no-sheets") { options.bNoSheets = true; }
        else if (arg == "--sheet-preview")
        {
            if (!next(value)) { return 2; }
            options.sheetPreview = value;
        }
        else if (arg == "--sheet-preview-scale")
        {
            if (!nextInt(options.sheetPreviewScale)) { return 2; }
            if (options.sheetPreviewScale < 1) { options.sheetPreviewScale = 1; }
        }
        else if (arg == "--threads") { if (!nextInt(options.threads)) { return 2; } }
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
