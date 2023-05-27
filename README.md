# PTCR 2.0
![20230527_161632_3270SPP_](https://github.com/Panjaksli/PTCR2.0/assets/82727531/62732e90-a41e-4346-9fbc-b35a8d27d9a5)
## Requirements
CPU with **at least** SSE 4.1 support. <br>
Release version is compiled with -march=haswell. <br>
Thus only Haswell or AMD equivalent architectures and newer are supported natively. <br>
For older CPUs (Sandy/Ivy bridge), replace **-march=haswell** by **-march=native** in project properties and compile yourself.
![image](https://user-images.githubusercontent.com/82727531/196033712-3999a534-bf3d-4428-95ea-3b96ab3b11fe.png)

## Compile instructions:
1) Requires **Visual Studio 202x** version with **Clang compiler** installed !!!
2) Instructions for adding Clang to VS here:<br>
https://learn.microsoft.com/en-us/cpp/build/clang-support-msbuild?view=msvc-170
3) Open .sln file and select "Release" configuration
4) Compile and run
5) You can also use the precompiled version in "Releases" github section (only Haswell and newer).


## Požadavky
Procesor **alespoň** s podporou instrukční sady SSE 4.1. <br>
Konfigurace "Release" je zkompilována s nastavením -march=haswell. <br>
Tudíž jsou podporovány pouze procesory architektury haswell, nebo AMD ekvivalenty. <br>
Pro starší generace procesorů (Sandy/Ivy bridge), je nutno nahradit **-march=haswell** příznakem **-march=native** a musí být ručně rekompilován.
![image](https://user-images.githubusercontent.com/82727531/196033712-3999a534-bf3d-4428-95ea-3b96ab3b11fe.png)

## Instrukce ke kompilaci:
1) Požaduje verzi **Visual Studio 202x** s nainstalovaným balíčkem **Clang** !!!
2) Instrukce pro přidání Clang do VS lze nalézt zde:<br>
https://learn.microsoft.com/en-us/cpp/build/clang-support-msbuild?view=msvc-170
3) Otevřete .sln soubor v hlavním adresáři, zvolte horní nabídce konfiguraci "Release"
4) Klikněte na tlačítko "Compile and run"
5) Prekompilovaná verze se nachází v položce "Releases" (pouze Haswell a novější).
