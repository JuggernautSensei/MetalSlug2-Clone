# Visual Studio 솔루션/프로젝트 파일을 Source/, Test/ 의 현재 파일 목록으로 다시 생성한다.
# 파일을 추가/삭제/이동한 뒤 이 스크립트를 실행하고, 생성된 MyMetalSlug2.sln, Projects/**/*.vcxproj(.filters) 를 함께 커밋한다.
# 사용: powershell -NoProfile -ExecutionPolicy Bypass -File Scripts/GenerateProjects.ps1
# 출력은 항상 정렬되어 같은 입력이면 같은 결과가 나온다. 프로젝트 GUID 는 고정값이다(바꾸지 말 것).
param()

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$enc  = New-Object System.Text.UTF8Encoding($true)

$kTypeGuid = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'
$kVcpkgSystemLibs = 'SDL3-static.lib;yyjson.lib;kernel32.lib;user32.lib;gdi32.lib;winmm.lib;imm32.lib;ole32.lib;oleaut32.lib;version.lib;uuid.lib;advapi32.lib;setupapi.lib;shell32.lib;dinput8.lib'

# 프로젝트 정의. Dirs: 소스를 찾을 폴더(재귀 여부), Exclude: 제외할 상대 경로 정규식.
$projects = @(
    [pscustomobject]@{ Name = 'MyMetalSlug2'; Guid = '{0441C2CE-2EAA-4887-9B2A-BC03B24C69BD}'; Kind = 'Application'; Warn = 3; AssetDefine = $false; Refs = @('SlugCore'); Libs = $true;  Dirs = @(@{ Path = 'Source';            Recurse = $false; Files = @('Main.cpp') }) }
    [pscustomobject]@{ Name = 'SlugCore';      Guid = '{ABF44F8E-4D16-463A-9FC7-E2085E9810E3}'; Kind = 'StaticLibrary'; Warn = 4; AssetDefine = $false; Refs = @(); Libs = $false; Dirs = @(@{ Path = 'Source'; Recurse = $true; Exclude = '^Source[\\/](Main\.cpp|Tools[\\/])' }) }
    [pscustomobject]@{ Name = 'SlugTest';      Guid = '{A8A1C747-F01F-452B-AC29-33D90D36896A}'; Kind = 'Application'; Warn = 3; AssetDefine = $true;  Refs = @('SlugCore'); Libs = $true; ExtraLibs = 'gtest.lib'; ExtraIncludes = @('$(SolutionDir)Test'); Dirs = @(@{ Path = 'Test'; Recurse = $false }) }
    [pscustomobject]@{ Name = 'SlugBaker';     Guid = '{3B5EEBC8-E12D-4B7B-8F35-8E6BEED0D691}'; Kind = 'Application'; Warn = 3; AssetDefine = $true;  Refs = @('SlugCore'); Libs = $true; Dirs = @(@{ Path = 'Source\Tools\Baker'; Recurse = $false }) }
    [pscustomobject]@{ Name = 'SlugHeadless';  Guid = '{C6565C76-5E0E-4213-A131-93F5C939BD08}'; Kind = 'Application'; Warn = 3; AssetDefine = $true;  Refs = @('SlugCore'); Libs = $true; Dirs = @(@{ Path = 'Source\Tools\Headless'; Recurse = $false }) }
)

function Get-ProjectGuid([string]$name)
{
    return ($projects | Where-Object { $_.Name -eq $name }).Guid
}

function Get-DeterministicGuid([string]$seed)
{
    $md5 = [System.Security.Cryptography.MD5]::Create()
    $bytes = $md5.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($seed))
    return ('{' + ([guid]::new($bytes)).ToString().ToUpper() + '}')
}

function Write-TextFile([string]$path, [string]$text)
{
    $dir = Split-Path -Parent $path
    if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir | Out-Null }
    $crlf = $text -replace "`r`n", "`n" -replace "`n", "`r`n"
    [System.IO.File]::WriteAllText($path, $crlf, $enc)
}

# 프로젝트가 가진 소스 목록 [{ Rel = 'Source\Core\App.cpp' (저장소 기준), Kind = 'ClCompile'|'ClInclude' }] 을 정렬해서 돌려준다.
function Get-ProjectFiles($project)
{
    $items = New-Object System.Collections.Generic.List[object]
    foreach ($dirSpec in $project.Dirs)
    {
        $dirPath = Join-Path $root $dirSpec.Path
        $found = Get-ChildItem -LiteralPath $dirPath -File -Recurse:([bool]$dirSpec.Recurse) | Where-Object { $_.Extension -eq '.cpp' -or $_.Extension -eq '.h' }
        foreach ($file in $found)
        {
            $rel = $file.FullName.Substring($root.Length + 1)
            if ($dirSpec.Exclude -and ($rel -match $dirSpec.Exclude)) { continue }
            if ($dirSpec.Files -and ($dirSpec.Files -notcontains $file.Name)) { continue }
            $kind = if ($file.Extension -eq '.cpp') { 'ClCompile' } else { 'ClInclude' }
            $items.Add([pscustomobject]@{ Rel = $rel; Kind = $kind })
        }
    }
    return @($items | Sort-Object -Property @{ Expression = { $_.Rel.ToLowerInvariant() } })
}

function Get-VcxprojText($project, $files)
{
    $guid = $project.Guid
    $name = $project.Name
    $isExe = $project.Kind -eq 'Application'
    $warn = "Level$($project.Warn)"

    $defsDebug = '_DEBUG;NOMINMAX;UNICODE;_UNICODE'
    $defsRelease = 'NDEBUG;NOMINMAX;UNICODE;_UNICODE'
    if ($project.AssetDefine)
    {
        $assetDef = 'SLUG_ASSET_DIR="$(SolutionDir.Replace(''\'', ''/''))Assets"'
        $defsDebug = "$defsDebug;$assetDef"
        $defsRelease = "$defsRelease;$assetDef"
    }

    $extraInc = ''
    foreach ($inc in $project.ExtraIncludes) { $extraInc += "$inc;" }
    $incDirs = "$extraInc`$(SolutionDir)Source"

    $libs = ''
    if ($project.Libs)
    {
        $libs = $kVcpkgSystemLibs
        if ($project.ExtraLibs) { $libs = "$($project.ExtraLibs);$libs" }
    }

    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine('<?xml version="1.0" encoding="utf-8"?>')
    [void]$sb.AppendLine('<Project DefaultTargets="Build" ToolsVersion="17.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">')
    [void]$sb.AppendLine('  <!-- 자동 생성: Scripts/GenerateProjects.ps1 . 직접 고치지 말고 스크립트를 다시 실행할 것. -->')
    [void]$sb.AppendLine('  <ItemGroup Label="ProjectConfigurations">')
    foreach ($cfg in 'Debug', 'Release')
    {
        [void]$sb.AppendLine("    <ProjectConfiguration Include=`"$cfg|x64`">")
        [void]$sb.AppendLine("      <Configuration>$cfg</Configuration>")
        [void]$sb.AppendLine('      <Platform>x64</Platform>')
        [void]$sb.AppendLine('    </ProjectConfiguration>')
    }
    [void]$sb.AppendLine('  </ItemGroup>')
    [void]$sb.AppendLine('  <PropertyGroup Label="Globals">')
    [void]$sb.AppendLine('    <VCProjectVersion>17.0</VCProjectVersion>')
    [void]$sb.AppendLine("    <ProjectGuid>$guid</ProjectGuid>")
    [void]$sb.AppendLine("    <RootNamespace>$name</RootNamespace>")
    [void]$sb.AppendLine("    <ProjectName>$name</ProjectName>")
    [void]$sb.AppendLine('    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>')
    [void]$sb.AppendLine('    <Keyword>Win32Proj</Keyword>')
    [void]$sb.AppendLine('  </PropertyGroup>')
    [void]$sb.AppendLine('  <PropertyGroup>')
    [void]$sb.AppendLine('    <!-- 솔루션 밖에서 프로젝트만 열어 빌드해도 경로가 맞도록 기본값을 둔다 (sln 빌드 시에는 MSBuild 가 넘겨준 값을 쓴다). -->')
    [void]$sb.AppendLine('    <SolutionDir Condition="''$(SolutionDir)'' == ''''">$(MSBuildThisFileDirectory)..\..\</SolutionDir>')
    [void]$sb.AppendLine('  </PropertyGroup>')
    [void]$sb.AppendLine('  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />')
    foreach ($cfg in 'Debug', 'Release')
    {
        $useDbg = if ($cfg -eq 'Debug') { 'true' } else { 'false' }
        $cfgType = $project.Kind
        [void]$sb.AppendLine("  <PropertyGroup Condition=`"'`$(Configuration)|`$(Platform)'=='$cfg|x64'`" Label=`"Configuration`">")
        [void]$sb.AppendLine("    <ConfigurationType>$cfgType</ConfigurationType>")
        [void]$sb.AppendLine("    <UseDebugLibraries>$useDbg</UseDebugLibraries>")
        [void]$sb.AppendLine('    <PlatformToolset>v145</PlatformToolset>')
        [void]$sb.AppendLine('    <CharacterSet>Unicode</CharacterSet>')
        if ($cfg -eq 'Release') { [void]$sb.AppendLine('    <WholeProgramOptimization>false</WholeProgramOptimization>') }
        [void]$sb.AppendLine('  </PropertyGroup>')
    }
    [void]$sb.AppendLine('  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />')
    [void]$sb.AppendLine('  <PropertyGroup Label="Vcpkg">')
    [void]$sb.AppendLine('    <!-- vcpkg 매니페스트 모드(저장소 루트 vcpkg.json). VCPKG_ROOT 환경변수가 있으면 우선, 없으면 C:\vcpkg\ . -->')
    [void]$sb.AppendLine('    <VcpkgRoot Condition="''$(VcpkgRoot)'' == '''' And ''$(VCPKG_ROOT)'' != ''''">$([MSBuild]::EnsureTrailingSlash(''$(VCPKG_ROOT)''))</VcpkgRoot>')
    [void]$sb.AppendLine('    <VcpkgRoot Condition="''$(VcpkgRoot)'' == ''''">C:\vcpkg\</VcpkgRoot>')
    [void]$sb.AppendLine('    <VcpkgEnableManifest>true</VcpkgEnableManifest>')
    [void]$sb.AppendLine('    <VcpkgTriplet>x64-windows-static-md</VcpkgTriplet>')
    [void]$sb.AppendLine('  </PropertyGroup>')
    [void]$sb.AppendLine('  <!-- 전역 vcpkg integrate(vcpkg.user.targets)가 이미 있으면 그쪽이 가져오므로 중복 import 하지 않는다. -->')
    [void]$sb.AppendLine('  <Import Project="$(VcpkgRoot)scripts\buildsystems\msbuild\vcpkg.props" Condition="Exists(''$(VcpkgRoot)scripts\buildsystems\msbuild\vcpkg.props'') And !Exists(''$(LOCALAPPDATA)\vcpkg\vcpkg.user.targets'')" />')
    [void]$sb.AppendLine('  <ImportGroup Label="ExtensionSettings">')
    [void]$sb.AppendLine('  </ImportGroup>')
    [void]$sb.AppendLine('  <ImportGroup Label="Shared">')
    [void]$sb.AppendLine('  </ImportGroup>')
    [void]$sb.AppendLine('  <ImportGroup Label="PropertySheets">')
    [void]$sb.AppendLine('    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists(''$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props'')" Label="LocalAppDataPlatform" />')
    [void]$sb.AppendLine('  </ImportGroup>')
    [void]$sb.AppendLine('  <PropertyGroup Label="UserMacros" />')
    [void]$sb.AppendLine('  <PropertyGroup>')
    [void]$sb.AppendLine('    <OutDir>$(SolutionDir)Output\$(Configuration)\</OutDir>')
    [void]$sb.AppendLine('    <IntDir>$(SolutionDir)Intermediate\$(ProjectName)\$(Configuration)\</IntDir>')
    [void]$sb.AppendLine('  </PropertyGroup>')

    foreach ($cfg in 'Debug', 'Release')
    {
        [void]$sb.AppendLine("  <ItemDefinitionGroup Condition=`"'`$(Configuration)|`$(Platform)'=='$cfg|x64'`">")
        [void]$sb.AppendLine('    <ClCompile>')
        [void]$sb.AppendLine("      <WarningLevel>$warn</WarningLevel>")
        if ($cfg -eq 'Debug')
        {
            [void]$sb.AppendLine('      <Optimization>Disabled</Optimization>')
            [void]$sb.AppendLine("      <PreprocessorDefinitions>$defsDebug;%(PreprocessorDefinitions)</PreprocessorDefinitions>")
            [void]$sb.AppendLine('      <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>')
        }
        else
        {
            [void]$sb.AppendLine('      <Optimization>MaxSpeed</Optimization>')
            [void]$sb.AppendLine('      <FunctionLevelLinking>true</FunctionLevelLinking>')
            [void]$sb.AppendLine('      <IntrinsicFunctions>true</IntrinsicFunctions>')
            [void]$sb.AppendLine("      <PreprocessorDefinitions>$defsRelease;%(PreprocessorDefinitions)</PreprocessorDefinitions>")
            [void]$sb.AppendLine('      <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>')
        }
        [void]$sb.AppendLine('      <LanguageStandard>stdcpp20</LanguageStandard>')
        [void]$sb.AppendLine('      <ConformanceMode>true</ConformanceMode>')
        [void]$sb.AppendLine('      <UseStandardPreprocessor>true</UseStandardPreprocessor>')
        [void]$sb.AppendLine('      <MultiProcessorCompilation>true</MultiProcessorCompilation>')
        [void]$sb.AppendLine('      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>')
        [void]$sb.AppendLine("      <AdditionalIncludeDirectories>$incDirs;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>")
        [void]$sb.AppendLine('      <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>')
        [void]$sb.AppendLine('    </ClCompile>')
        if ($isExe)
        {
            [void]$sb.AppendLine('    <Link>')
            [void]$sb.AppendLine('      <SubSystem>Console</SubSystem>')
            [void]$sb.AppendLine('      <GenerateDebugInformation>true</GenerateDebugInformation>')
            if ($cfg -eq 'Release')
            {
                [void]$sb.AppendLine('      <EnableCOMDATFolding>true</EnableCOMDATFolding>')
                [void]$sb.AppendLine('      <OptimizeReferences>true</OptimizeReferences>')
            }
            if ($libs) { [void]$sb.AppendLine("      <AdditionalDependencies>$libs;%(AdditionalDependencies)</AdditionalDependencies>") }
            [void]$sb.AppendLine('    </Link>')
        }
        [void]$sb.AppendLine('  </ItemDefinitionGroup>')
    }

    $compile = @($files | Where-Object { $_.Kind -eq 'ClCompile' })
    $include = @($files | Where-Object { $_.Kind -eq 'ClInclude' })
    [void]$sb.AppendLine('  <ItemGroup>')
    foreach ($f in $compile) { [void]$sb.AppendLine("    <ClCompile Include=`"..\..\$($f.Rel)`" />") }
    [void]$sb.AppendLine('  </ItemGroup>')
    [void]$sb.AppendLine('  <ItemGroup>')
    foreach ($f in $include) { [void]$sb.AppendLine("    <ClInclude Include=`"..\..\$($f.Rel)`" />") }
    [void]$sb.AppendLine('  </ItemGroup>')
    if ($project.Refs.Count -gt 0)
    {
        [void]$sb.AppendLine('  <ItemGroup>')
        foreach ($ref in $project.Refs)
        {
            [void]$sb.AppendLine("    <ProjectReference Include=`"..\$ref\$ref.vcxproj`">")
            [void]$sb.AppendLine("      <Project>$(Get-ProjectGuid $ref)</Project>")
            [void]$sb.AppendLine('    </ProjectReference>')
        }
        [void]$sb.AppendLine('  </ItemGroup>')
    }
    [void]$sb.AppendLine('  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />')
    [void]$sb.AppendLine('  <Import Project="$(VcpkgRoot)scripts\buildsystems\msbuild\vcpkg.targets" Condition="Exists(''$(VcpkgRoot)scripts\buildsystems\msbuild\vcpkg.targets'') And !Exists(''$(LOCALAPPDATA)\vcpkg\vcpkg.user.targets'')" />')
    [void]$sb.AppendLine('  <ImportGroup Label="ExtensionTargets">')
    [void]$sb.AppendLine('  </ImportGroup>')
    [void]$sb.AppendLine('</Project>')
    return $sb.ToString()
}

function Get-FiltersText($project, $files)
{
    # 필터는 저장소 상대 폴더(Source\Core 등)를 그대로 쓰고, 상위 필터도 모두 선언한다.
    $filterOf = @{}
    $allFilters = New-Object System.Collections.Generic.SortedSet[string] ([StringComparer]::Ordinal)
    foreach ($f in $files)
    {
        $dir = Split-Path -Parent $f.Rel
        $filterOf[$f.Rel] = $dir
        $parts = $dir -split '\\'
        for ($i = 1; $i -le $parts.Count; $i++)
        {
            if ($parts[0..($i - 1)] -join '\') { [void]$allFilters.Add(($parts[0..($i - 1)] -join '\')) }
        }
    }

    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine('<?xml version="1.0" encoding="utf-8"?>')
    [void]$sb.AppendLine('<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">')
    [void]$sb.AppendLine('  <ItemGroup>')
    foreach ($flt in $allFilters)
    {
        [void]$sb.AppendLine("    <Filter Include=`"$flt`">")
        [void]$sb.AppendLine("      <UniqueIdentifier>$(Get-DeterministicGuid "$($project.Name)|$flt")</UniqueIdentifier>")
        [void]$sb.AppendLine('    </Filter>')
    }
    [void]$sb.AppendLine('  </ItemGroup>')
    foreach ($kind in 'ClCompile', 'ClInclude')
    {
        $group = @($files | Where-Object { $_.Kind -eq $kind })
        [void]$sb.AppendLine('  <ItemGroup>')
        foreach ($f in $group)
        {
            [void]$sb.AppendLine("    <$kind Include=`"..\..\$($f.Rel)`">")
            $flt = $filterOf[$f.Rel]
            if ($flt) { [void]$sb.AppendLine("      <Filter>$flt</Filter>") }
            [void]$sb.AppendLine("    </$kind>")
        }
        [void]$sb.AppendLine('  </ItemGroup>')
    }
    [void]$sb.AppendLine('</Project>')
    return $sb.ToString()
}

$summary = @()
foreach ($project in $projects)
{
    $files = Get-ProjectFiles $project
    $dir = Join-Path $root "Projects\$($project.Name)"
    Write-TextFile (Join-Path $dir "$($project.Name).vcxproj") (Get-VcxprojText $project $files)
    Write-TextFile (Join-Path $dir "$($project.Name).vcxproj.filters") (Get-FiltersText $project $files)
    $summary += "$($project.Name): $(@($files | Where-Object { $_.Kind -eq 'ClCompile' }).Count) cpp, $(@($files | Where-Object { $_.Kind -eq 'ClInclude' }).Count) h"
}

# 솔루션 파일 (MyMetalSlug2 가 첫 프로젝트 = 기본 시작 프로젝트)
$sln = New-Object System.Text.StringBuilder
[void]$sln.AppendLine('Microsoft Visual Studio Solution File, Format Version 12.00')
[void]$sln.AppendLine('# Visual Studio Version 17')
[void]$sln.AppendLine('VisualStudioVersion = 17.0.31903.59')
[void]$sln.AppendLine('MinimumVisualStudioVersion = 10.0.40219.1')
foreach ($project in $projects)
{
    $rel = "Projects\$($project.Name)\$($project.Name).vcxproj"
    [void]$sln.AppendLine("Project(`"$kTypeGuid`") = `"$($project.Name)`", `"$rel`", `"$($project.Guid)`"")
    [void]$sln.AppendLine('EndProject')
}
[void]$sln.AppendLine('Global')
[void]$sln.AppendLine('	GlobalSection(SolutionConfigurationPlatforms) = preSolution')
[void]$sln.AppendLine('		Debug|x64 = Debug|x64')
[void]$sln.AppendLine('		Release|x64 = Release|x64')
[void]$sln.AppendLine('	EndGlobalSection')
[void]$sln.AppendLine('	GlobalSection(ProjectConfigurationPlatforms) = postSolution')
foreach ($project in $projects)
{
    foreach ($cfg in 'Debug', 'Release')
    {
        [void]$sln.AppendLine("		$($project.Guid).$cfg|x64.ActiveCfg = $cfg|x64")
        [void]$sln.AppendLine("		$($project.Guid).$cfg|x64.Build.0 = $cfg|x64")
    }
}
[void]$sln.AppendLine('	EndGlobalSection')
[void]$sln.AppendLine('	GlobalSection(SolutionProperties) = preSolution')
[void]$sln.AppendLine('		HideSolutionNode = FALSE')
[void]$sln.AppendLine('	EndGlobalSection')
[void]$sln.AppendLine('EndGlobal')
Write-TextFile (Join-Path $root 'MyMetalSlug2.sln') $sln.ToString()

$summary | ForEach-Object { Write-Output $_ }
Write-Output 'generated MyMetalSlug2.sln and Projects\*'
