#!/usr/bin/env python3
# Visual Studio 솔루션/프로젝트 파일을 Source/, Test/ 의 현재 파일 목록으로 다시 생성한다.
# Scripts/GenerateProjects.ps1 과 같은 출력을 내는 이식본 (PowerShell 이 없는 Linux 클라우드 환경용).
# 사용: python3 Scripts/GenerateProjects.py
# 출력은 항상 정렬되어 같은 입력이면 같은 결과가 나온다. 프로젝트 GUID 는 고정값이다(바꾸지 말 것).
# 파일은 UTF-8 BOM + CRLF 로 쓴다 (.ps1 과 같다).

import hashlib
import re
import sys
import uuid
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

TYPE_GUID = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'
VCPKG_SYSTEM_LIBS = ('SDL3-static.lib;yyjson.lib;kernel32.lib;user32.lib;gdi32.lib;winmm.lib;imm32.lib;ole32.lib;'
                     'oleaut32.lib;version.lib;uuid.lib;advapi32.lib;setupapi.lib;shell32.lib;dinput8.lib')

# Dirs: (폴더, 재귀 여부, 포함 파일명 목록 또는 None, 제외 상대 경로 정규식 또는 None). 상대 경로는 역슬래시 기준.
PROJECTS = [
    dict(name='MyMetalSlug2', guid='{0441C2CE-2EAA-4887-9B2A-BC03B24C69BD}', kind='Application', warn=3, asset_define=False,
         refs=['SlugCore'], libs=True, extra_libs=None, extra_includes=[],
         dirs=[('Source', False, ['Main.cpp'], None)]),
    dict(name='SlugCore', guid='{ABF44F8E-4D16-463A-9FC7-E2085E9810E3}', kind='StaticLibrary', warn=4, asset_define=False,
         refs=[], libs=False, extra_libs=None, extra_includes=[],
         dirs=[('Source', True, None, r'^Source[\\/](Main\.cpp|Tools[\\/])')]),
    dict(name='SlugTest', guid='{A8A1C747-F01F-452B-AC29-33D90D36896A}', kind='Application', warn=3, asset_define=True,
         refs=['SlugCore'], libs=True, extra_libs='gtest.lib', extra_includes=['$(SolutionDir)Test'],
         dirs=[('Test', False, None, None)]),
    dict(name='SlugBaker', guid='{3B5EEBC8-E12D-4B7B-8F35-8E6BEED0D691}', kind='Application', warn=3, asset_define=True,
         refs=['SlugCore'], libs=True, extra_libs=None, extra_includes=[],
         dirs=[('Source/Tools/Baker', False, None, None)]),
    dict(name='SlugHeadless', guid='{C6565C76-5E0E-4213-A131-93F5C939BD08}', kind='Application', warn=3, asset_define=True,
         refs=['SlugCore'], libs=True, extra_libs=None, extra_includes=[],
         dirs=[('Source/Tools/Headless', False, None, None)]),
]


def get_project_guid(name):
    return next(p['guid'] for p in PROJECTS if p['name'] == name)


def get_deterministic_guid(seed):
    digest = hashlib.md5(seed.encode('utf-8')).digest()
    # .NET 의 new Guid(byte[]) 는 앞 3 필드를 리틀엔디언으로 읽는다
    return '{' + str(uuid.UUID(bytes_le=digest)).upper() + '}'


def write_text_file(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    crlf = text.replace('\r\n', '\n').replace('\n', '\r\n')
    path.write_bytes(b'\xef\xbb\xbf' + crlf.encode('utf-8'))


def get_project_files(project):
    """[(rel, kind)] 정렬 목록. rel 은 저장소 기준 역슬래시 경로, kind 는 ClCompile/ClInclude."""
    items = []
    for path, recurse, files, exclude in project['dirs']:
        dir_path = ROOT / path
        found = dir_path.rglob('*') if recurse else dir_path.glob('*')
        for file in found:
            if not file.is_file() or file.suffix.lower() not in ('.cpp', '.h'):
                continue
            rel = str(file.relative_to(ROOT)).replace('/', '\\')
            if exclude and re.search(exclude, rel):
                continue
            if files is not None and file.name not in files:
                continue
            items.append((rel, 'ClCompile' if file.suffix.lower() == '.cpp' else 'ClInclude'))
    items.sort(key=lambda item: item[0].lower())
    return items


def get_vcxproj_text(project, files):
    guid = project['guid']
    name = project['name']
    is_exe = project['kind'] == 'Application'
    warn = 'Level%d' % project['warn']

    defs_debug = '_DEBUG;NOMINMAX;UNICODE;_UNICODE'
    defs_release = 'NDEBUG;NOMINMAX;UNICODE;_UNICODE'
    if project['asset_define']:
        asset_def = 'SLUG_ASSET_DIR="$(SolutionDir.Replace(\'\\\', \'/\'))Assets"'
        defs_debug = defs_debug + ';' + asset_def
        defs_release = defs_release + ';' + asset_def

    extra_inc = ''.join(inc + ';' for inc in project['extra_includes'])
    inc_dirs = extra_inc + '$(SolutionDir)Source'

    libs = ''
    if project['libs']:
        libs = VCPKG_SYSTEM_LIBS
        if project['extra_libs']:
            libs = project['extra_libs'] + ';' + libs

    out = []
    add = out.append
    add('<?xml version="1.0" encoding="utf-8"?>')
    add('<Project DefaultTargets="Build" ToolsVersion="17.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">')
    add('  <!-- 자동 생성: Scripts/GenerateProjects.ps1 . 직접 고치지 말고 스크립트를 다시 실행할 것. -->')
    add('  <ItemGroup Label="ProjectConfigurations">')
    for cfg in ('Debug', 'Release'):
        add('    <ProjectConfiguration Include="%s|x64">' % cfg)
        add('      <Configuration>%s</Configuration>' % cfg)
        add('      <Platform>x64</Platform>')
        add('    </ProjectConfiguration>')
    add('  </ItemGroup>')
    add('  <PropertyGroup Label="Globals">')
    add('    <VCProjectVersion>17.0</VCProjectVersion>')
    add('    <ProjectGuid>%s</ProjectGuid>' % guid)
    add('    <RootNamespace>%s</RootNamespace>' % name)
    add('    <ProjectName>%s</ProjectName>' % name)
    add('    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>')
    add('    <Keyword>Win32Proj</Keyword>')
    add('  </PropertyGroup>')
    add('  <PropertyGroup>')
    add('    <!-- 솔루션 밖에서 프로젝트만 열어 빌드해도 경로가 맞도록 기본값을 둔다 (sln 빌드 시에는 MSBuild 가 넘겨준 값을 쓴다). -->')
    add('    <SolutionDir Condition="\'$(SolutionDir)\' == \'\'">$(MSBuildThisFileDirectory)..\\..\\</SolutionDir>')
    add('  </PropertyGroup>')
    add('  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />')
    for cfg in ('Debug', 'Release'):
        use_dbg = 'true' if cfg == 'Debug' else 'false'
        add('  <PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'%s|x64\'" Label="Configuration">' % cfg)
        add('    <ConfigurationType>%s</ConfigurationType>' % project['kind'])
        add('    <UseDebugLibraries>%s</UseDebugLibraries>' % use_dbg)
        add('    <PlatformToolset>v145</PlatformToolset>')
        add('    <CharacterSet>Unicode</CharacterSet>')
        if cfg == 'Release':
            add('    <WholeProgramOptimization>false</WholeProgramOptimization>')
        add('  </PropertyGroup>')
    add('  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />')
    add('  <PropertyGroup Label="Vcpkg">')
    add('    <!-- vcpkg 매니페스트 모드(저장소 루트 vcpkg.json). VCPKG_ROOT 환경변수가 있으면 우선, 없으면 C:\\vcpkg\\ . -->')
    add('    <VcpkgRoot Condition="\'$(VcpkgRoot)\' == \'\' And \'$(VCPKG_ROOT)\' != \'\'">$([MSBuild]::EnsureTrailingSlash(\'$(VCPKG_ROOT)\'))</VcpkgRoot>')
    add('    <VcpkgRoot Condition="\'$(VcpkgRoot)\' == \'\'">C:\\vcpkg\\</VcpkgRoot>')
    add('    <VcpkgEnableManifest>true</VcpkgEnableManifest>')
    add('    <VcpkgTriplet>x64-windows-static-md</VcpkgTriplet>')
    add('  </PropertyGroup>')
    add('  <!-- 전역 vcpkg integrate(vcpkg.user.targets)가 이미 있으면 그쪽이 가져오므로 중복 import 하지 않는다. -->')
    add('  <Import Project="$(VcpkgRoot)scripts\\buildsystems\\msbuild\\vcpkg.props" Condition="Exists(\'$(VcpkgRoot)scripts\\buildsystems\\msbuild\\vcpkg.props\') And !Exists(\'$(LOCALAPPDATA)\\vcpkg\\vcpkg.user.targets\')" />')
    add('  <ImportGroup Label="ExtensionSettings">')
    add('  </ImportGroup>')
    add('  <ImportGroup Label="Shared">')
    add('  </ImportGroup>')
    add('  <ImportGroup Label="PropertySheets">')
    add('    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />')
    add('  </ImportGroup>')
    add('  <PropertyGroup Label="UserMacros" />')
    add('  <PropertyGroup>')
    add('    <OutDir>$(SolutionDir)Output\\$(Configuration)\\</OutDir>')
    add('    <IntDir>$(SolutionDir)Intermediate\\$(ProjectName)\\$(Configuration)\\</IntDir>')
    add('  </PropertyGroup>')

    for cfg in ('Debug', 'Release'):
        add('  <ItemDefinitionGroup Condition="\'$(Configuration)|$(Platform)\'==\'%s|x64\'">' % cfg)
        add('    <ClCompile>')
        add('      <WarningLevel>%s</WarningLevel>' % warn)
        if cfg == 'Debug':
            add('      <Optimization>Disabled</Optimization>')
            add('      <PreprocessorDefinitions>%s;%%(PreprocessorDefinitions)</PreprocessorDefinitions>' % defs_debug)
            add('      <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>')
        else:
            add('      <Optimization>MaxSpeed</Optimization>')
            add('      <FunctionLevelLinking>true</FunctionLevelLinking>')
            add('      <IntrinsicFunctions>true</IntrinsicFunctions>')
            add('      <PreprocessorDefinitions>%s;%%(PreprocessorDefinitions)</PreprocessorDefinitions>' % defs_release)
            add('      <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>')
        add('      <LanguageStandard>stdcpp20</LanguageStandard>')
        add('      <ConformanceMode>true</ConformanceMode>')
        add('      <UseStandardPreprocessor>true</UseStandardPreprocessor>')
        add('      <MultiProcessorCompilation>true</MultiProcessorCompilation>')
        add('      <AdditionalOptions>/utf-8 %(AdditionalOptions)</AdditionalOptions>')
        add('      <AdditionalIncludeDirectories>%s;%%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>' % inc_dirs)
        add('      <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>')
        add('    </ClCompile>')
        if is_exe:
            add('    <Link>')
            add('      <SubSystem>Console</SubSystem>')
            add('      <GenerateDebugInformation>true</GenerateDebugInformation>')
            if cfg == 'Release':
                add('      <EnableCOMDATFolding>true</EnableCOMDATFolding>')
                add('      <OptimizeReferences>true</OptimizeReferences>')
            if libs:
                add('      <AdditionalDependencies>%s;%%(AdditionalDependencies)</AdditionalDependencies>' % libs)
            add('    </Link>')
        add('  </ItemDefinitionGroup>')

    compile_files = [f for f in files if f[1] == 'ClCompile']
    include_files = [f for f in files if f[1] == 'ClInclude']
    add('  <ItemGroup>')
    for rel, _ in compile_files:
        add('    <ClCompile Include="..\\..\\%s" />' % rel)
    add('  </ItemGroup>')
    add('  <ItemGroup>')
    for rel, _ in include_files:
        add('    <ClInclude Include="..\\..\\%s" />' % rel)
    add('  </ItemGroup>')
    if project['refs']:
        add('  <ItemGroup>')
        for ref in project['refs']:
            add('    <ProjectReference Include="..\\%s\\%s.vcxproj">' % (ref, ref))
            add('      <Project>%s</Project>' % get_project_guid(ref))
            add('    </ProjectReference>')
        add('  </ItemGroup>')
    add('  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />')
    add('  <Import Project="$(VcpkgRoot)scripts\\buildsystems\\msbuild\\vcpkg.targets" Condition="Exists(\'$(VcpkgRoot)scripts\\buildsystems\\msbuild\\vcpkg.targets\') And !Exists(\'$(LOCALAPPDATA)\\vcpkg\\vcpkg.user.targets\')" />')
    add('  <ImportGroup Label="ExtensionTargets">')
    add('  </ImportGroup>')
    add('</Project>')
    return '\n'.join(out) + '\n'


def get_filters_text(project, files):
    # 필터는 저장소 상대 폴더(Source\Core 등)를 그대로 쓰고, 상위 필터도 모두 선언한다.
    filter_of = {}
    all_filters = set()
    for rel, _ in files:
        directory = rel.rpartition('\\')[0]
        filter_of[rel] = directory
        parts = directory.split('\\')
        for i in range(1, len(parts) + 1):
            joined = '\\'.join(parts[:i])
            if joined:
                all_filters.add(joined)

    out = []
    add = out.append
    add('<?xml version="1.0" encoding="utf-8"?>')
    add('<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">')
    add('  <ItemGroup>')
    for flt in sorted(all_filters):
        add('    <Filter Include="%s">' % flt)
        add('      <UniqueIdentifier>%s</UniqueIdentifier>' % get_deterministic_guid('%s|%s' % (project['name'], flt)))
        add('    </Filter>')
    add('  </ItemGroup>')
    for kind in ('ClCompile', 'ClInclude'):
        add('  <ItemGroup>')
        for rel, file_kind in files:
            if file_kind != kind:
                continue
            add('    <%s Include="..\\..\\%s">' % (kind, rel))
            flt = filter_of[rel]
            if flt:
                add('      <Filter>%s</Filter>' % flt)
            add('    </%s>' % kind)
        add('  </ItemGroup>')
    add('</Project>')
    return '\n'.join(out) + '\n'


def get_solution_text():
    out = []
    add = out.append
    add('Microsoft Visual Studio Solution File, Format Version 12.00')
    add('# Visual Studio Version 17')
    add('VisualStudioVersion = 17.0.31903.59')
    add('MinimumVisualStudioVersion = 10.0.40219.1')
    for project in PROJECTS:
        rel = 'Projects\\%s\\%s.vcxproj' % (project['name'], project['name'])
        add('Project("%s") = "%s", "%s", "%s"' % (TYPE_GUID, project['name'], rel, project['guid']))
        add('EndProject')
    add('Global')
    add('\tGlobalSection(SolutionConfigurationPlatforms) = preSolution')
    add('\t\tDebug|x64 = Debug|x64')
    add('\t\tRelease|x64 = Release|x64')
    add('\tEndGlobalSection')
    add('\tGlobalSection(ProjectConfigurationPlatforms) = postSolution')
    for project in PROJECTS:
        for cfg in ('Debug', 'Release'):
            add('\t\t%s.%s|x64.ActiveCfg = %s|x64' % (project['guid'], cfg, cfg))
            add('\t\t%s.%s|x64.Build.0 = %s|x64' % (project['guid'], cfg, cfg))
    add('\tEndGlobalSection')
    add('\tGlobalSection(SolutionProperties) = preSolution')
    add('\t\tHideSolutionNode = FALSE')
    add('\tEndGlobalSection')
    add('EndGlobal')
    return '\n'.join(out) + '\n'


def main():
    summary = []
    for project in PROJECTS:
        files = get_project_files(project)
        directory = ROOT / 'Projects' / project['name']
        write_text_file(directory / ('%s.vcxproj' % project['name']), get_vcxproj_text(project, files))
        write_text_file(directory / ('%s.vcxproj.filters' % project['name']), get_filters_text(project, files))
        cpp = sum(1 for f in files if f[1] == 'ClCompile')
        summary.append('%s: %d cpp, %d h' % (project['name'], cpp, len(files) - cpp))
    write_text_file(ROOT / 'MyMetalSlug2.sln', get_solution_text())
    for line in summary:
        print(line)
    print('generated MyMetalSlug2.sln and Projects/*')
    return 0


if __name__ == '__main__':
    sys.exit(main())
