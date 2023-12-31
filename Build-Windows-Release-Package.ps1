Param(
    # パッケージに付加するバージョン
    [parameter(Mandatory = $true)][string]$Version
)

# とりあえず Visual Studio Community 2019 用の MSBuild.exe にパスを通す
# 他の環境でビルドを実行する方法は要調査・検討
$msbuild_path = 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin'
$Env:Path = $msbuild_path + ";" + $Env:Path


function BuildPackage ($package_name, $package_unique_files, $build_conf) {
    # バイナリをリビルド
    MSBuild.exe .\Tangband\Tangband.sln /t:Rebuild /p:Configuration=$build_conf

    if ($LASTEXITCODE -ne 0) {
        # ビルド失敗ならスクリプトを中断する
        exit
    }

    # 作業用テンポラリフォルダ
    $tempDir = New-TemporaryFile | ForEach-Object { Remove-Item $_; New-Item $_ -ItemType Directory }

    $tangbandDir = Join-Path $tempDir $package_name
    New-Item $tangbandDir -ItemType Directory

    # 必要なファイルをコピーして、その中で不要になりえるものを削除
    Copy-Item -Verbose -Path .\Tangband.exe, .\readme_angband, .\THIRD-PARTY-NOTICES.txt -Destination $tangbandDir
    Copy-Item -Verbose -Path $package_unique_files -Destination $tangbandDir
    Copy-Item -Verbose -Recurse -Path .\lib -Destination $tangbandDir -Exclude Makefile.am, *.raw, .gitattributes
    Copy-Item -Verbose -Path .\lib\apex\h_scores.raw -Destination $tangbandDir\lib\apex
    Remove-Item -Verbose -Exclude delete.me -Recurse -Path $tangbandDir\lib\save\*, $tangbandDir\lib\user\*
    Remove-Item -Verbose -Exclude music.cfg, readme.txt, *.mp3 -Path $tangbandDir\lib\xtra\music\*

    # zipアーカイブ作成
    $package_path = Join-Path $(Get-Location) "${package_name}.zip"
    Get-ChildItem -Path $tempDir | Compress-Archive -Force -Verbose -DestinationPath $package_path

    # 作業用テンポラリフォルダ削除
    $tempDir | Where-Object { Test-Path $_ } | ForEach-Object { Get-ChildItem $_ -File -Recurse | Remove-Item; $_ } | Remove-Item -Recurse -Force
}

# 日本語版
BuildPackage -package_name Tangband-$Version-jp -package_unique_files .\readme.md, .\autopick.txt -build_conf Release
# 英語版
BuildPackage -package_name Tangband-$Version-en -package_unique_files .\readme-eng.md, .\autopick_eng.txt -build_conf English-Release
