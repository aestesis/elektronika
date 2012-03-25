; -- Example1.iss --
; Demonstrates copying 3 files and creating an icon.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=Aestesis Elektronika Live
AppId=Elektronika Live
AppVerName=Aestesis Elektronika Live 2.57 (vista)
DefaultDirName={pf}\aestesis\elektronika live
DefaultGroupName=Elektronika Live
UninstallDisplayIcon={app}\elektronika.exe
Compression=lzma
SolidCompression=yes
LicenseFile=license.txt
WizardImageFile=D:\aestesis\code\elektroSDK\elektronika\installer-icon-1.bmp
BackColor=$0080ff
BackColor2=clYellow
WizardSmallImageFile=D:\aestesis\code\elektroSDK\elektronika\installer-icon-2.bmp
WizardImageStretch=false

[Files]

; snaps
Source: snaps\readme.txt; DestDir: {app}\snaps

; bin
Source: bin\null.swf; DestDir: {app}\bin

; extra
Source: extras\XSkinExp.dle; DestDir: {app}\extras
Source: extras\readme.txt; DestDir: {app}\extras

; freeframe
Source: freeframe\AlphaTrail.dll; DestDir: {app}\freeframe
Source: freeframe\balls.bmp; DestDir: {app}\freeframe
Source: freeframe\Blockgenerator.dll; DestDir: {app}\freeframe
Source: freeframe\Blow.dll; DestDir: {app}\freeframe
Source: freeframe\ChristmasBalls.dll; DestDir: {app}\freeframe
Source: freeframe\Colorfilter.dll; DestDir: {app}\freeframe
Source: freeframe\DelayStrips.dll; DestDir: {app}\freeframe
Source: freeframe\EffectsStrips.dll; DestDir: {app}\freeframe
Source: freeframe\Ellipsegenerator.dll; DestDir: {app}\freeframe
Source: freeframe\Filmstrip.dll; DestDir: {app}\freeframe
Source: freeframe\FlitStrips.dll; DestDir: {app}\freeframe
Source: freeframe\FlitStripsV.dll; DestDir: {app}\freeframe
Source: freeframe\GlowDark.dll; DestDir: {app}\freeframe
Source: freeframe\lomo.dll; DestDir: {app}\freeframe
Source: freeframe\LomoV.dll; DestDir: {app}\freeframe
Source: freeframe\luma16.bmp; DestDir: {app}\freeframe
Source: freeframe\LumaBlow.dll; DestDir: {app}\freeframe
Source: freeframe\LumaEdges.dll; DestDir: {app}\freeframe
Source: freeframe\LumaImage.dll; DestDir: {app}\freeframe
Source: freeframe\LumaImageA.dll; DestDir: {app}\freeframe
Source: freeframe\LumaTrail.dll; DestDir: {app}\freeframe
Source: freeframe\MirrorStrips.dll; DestDir: {app}\freeframe
Source: freeframe\MirrorStripsV.dll; DestDir: {app}\freeframe
Source: freeframe\mmHshift.dll; DestDir: {app}\freeframe
Source: freeframe\mmVhold.dll; DestDir: {app}\freeframe
Source: freeframe\MultipleStripsNEW.dll; DestDir: {app}\freeframe
Source: freeframe\PeteBackDropKey.dll; DestDir: {app}\freeframe
Source: freeframe\PeteBackLight.dll; DestDir: {app}\freeframe
Source: freeframe\PeteBloom.dll; DestDir: {app}\freeframe
Source: freeframe\PeteBurn.dll; DestDir: {app}\freeframe
Source: freeframe\PeteChromium.dll; DestDir: {app}\freeframe
Source: freeframe\PeteColourReduce.dll; DestDir: {app}\freeframe
Source: freeframe\PeteColourWarp.dll; DestDir: {app}\freeframe
Source: freeframe\PeteDeInterlace.dll; DestDir: {app}\freeframe
Source: freeframe\PeteDilate.dll; DestDir: {app}\freeframe
Source: freeframe\PeteDirectionalBlur.dll; DestDir: {app}\freeframe
Source: freeframe\PeteDodge.dll; DestDir: {app}\freeframe
Source: freeframe\PeteExclusion.dll; DestDir: {app}\freeframe
Source: freeframe\PeteFishEye.dll; DestDir: {app}\freeframe
Source: freeframe\PeteGaussianBlur.dll; DestDir: {app}\freeframe
Source: freeframe\PeteGlow.dll; DestDir: {app}\freeframe
Source: freeframe\PeteHalfTone.dll; DestDir: {app}\freeframe
Source: freeframe\PeteKaleidascope.dll; DestDir: {app}\freeframe
Source: freeframe\PeteLevels.dll; DestDir: {app}\freeframe
Source: freeframe\PeteLionelBlur.dll; DestDir: {app}\freeframe
Source: freeframe\PeteLumaCycle.dll; DestDir: {app}\freeframe
Source: freeframe\PeteLumaKey.dll; DestDir: {app}\freeframe
Source: freeframe\PeteLumaOffset.dll; DestDir: {app}\freeframe
Source: freeframe\PeteMetaImage.dll; DestDir: {app}\freeframe
Source: freeframe\PetePanSpinZoom.dll; DestDir: {app}\freeframe
Source: freeframe\PetePosterize.dll; DestDir: {app}\freeframe
Source: freeframe\PeteRectDistort.dll; DestDir: {app}\freeframe
Source: freeframe\PeteRefraction.dll; DestDir: {app}\freeframe
Source: freeframe\PeteSlide.dll; DestDir: {app}\freeframe
Source: freeframe\PeteSmear.dll; DestDir: {app}\freeframe
Source: freeframe\PeteSolarize.dll; DestDir: {app}\freeframe
Source: freeframe\PeteSpiralBlur.dll; DestDir: {app}\freeframe
Source: freeframe\PeteSquash.dll; DestDir: {app}\freeframe
Source: freeframe\PeteStatic.dll; DestDir: {app}\freeframe
Source: freeframe\PeteStretch.dll; DestDir: {app}\freeframe
Source: freeframe\PeteTile.dll; DestDir: {app}\freeframe
Source: freeframe\PeteTimeBlur.dll; DestDir: {app}\freeframe
Source: freeframe\PeteTimeSlice.dll; DestDir: {app}\freeframe
Source: freeframe\PeteTimeWarp.dll; DestDir: {app}\freeframe
Source: freeframe\PeteTVPixel.dll; DestDir: {app}\freeframe
Source: freeframe\PeteVectorize.dll; DestDir: {app}\freeframe
Source: freeframe\PeteWave.dll; DestDir: {app}\freeframe
Source: freeframe\Puzzle.dll; DestDir: {app}\freeframe
Source: freeframe\randombuffer.dll; DestDir: {app}\freeframe
Source: freeframe\RearrangeStrips.dll; DestDir: {app}\freeframe
Source: freeframe\ScrollingStrip.dll; DestDir: {app}\freeframe
Source: freeframe\Slice.dll; DestDir: {app}\freeframe
Source: freeframe\SlidingStrips.dll; DestDir: {app}\freeframe
Source: freeframe\SlidingStripsBothWays.dll; DestDir: {app}\freeframe
Source: freeframe\SlidingStripsV.dll; DestDir: {app}\freeframe
Source: freeframe\Tv.dll; DestDir: {app}\freeframe
Source: freeframe\yoy-mosaic.dll; DestDir: {app}\freeframe

;help
Source: help\elektronika.chm; DestDir: {app}\help

; movies
Source: movies\fashion-01.avi; DestDir: {app}\movies
Source: movies\fashion-02.avi; DestDir: {app}\movies
Source: movies\fashion-03.avi; DestDir: {app}\movies
Source: movies\fashion-04.avi; DestDir: {app}\movies
Source: movies\fashion-05.avi; DestDir: {app}\movies
Source: movies\fashion-06.avi; DestDir: {app}\movies
Source: movies\daft punk.avi; DestDir: {app}\movies
Source: movies\japan-mini-tools.avi; DestDir: {app}\movies
Source: movies\pleix.avi; DestDir: {app}\movies
Source: movies\animatrix-010_qt.mov; DestDir: {app}\movies
Source: movies\animatrix-011_qt.mov; DestDir: {app}\movies
Source: movies\animatrix-012_qt.mov; DestDir: {app}\movies
Source: movies\animatrix-013_qt.mov; DestDir: {app}\movies
Source: movies\mire320.png; DestDir: {app}\movies

; movies/X
Source: movies\X\man-01.x; DestDir: {app}\movies\X
Source: movies\X\plante-01.x; DestDir: {app}\movies\X
Source: movies\X\pod.x; DestDir: {app}\movies\X
Source: movies\X\satelite.x; DestDir: {app}\movies\X
Source: movies\X\spacecraft2.x; DestDir: {app}\movies\X
Source: movies\X\spacecraft.x; DestDir: {app}\movies\X

; movies\flash
Source: movies\flash\bigMask.swf; DestDir: {app}\movies\flash
Source: movies\flash\elektronika.swf; DestDir: {app}\movies\flash
Source: movies\flash\flyingText.swf; DestDir: {app}\movies\flash
Source: movies\flash\glitch.swf; DestDir: {app}\movies\flash
Source: movies\flash\hacker2.swf; DestDir: {app}\movies\flash
Source: movies\flash\motionSwirl.swf; DestDir: {app}\movies\flash
Source: movies\flash\randomXYtext.swf; DestDir: {app}\movies\flash
Source: movies\flash\Rectangles-text-01.swf; DestDir: {app}\movies\flash
Source: movies\flash\ScrollingText.swf; DestDir: {app}\movies\flash
Source: movies\flash\this_is_aestesis.swf; DestDir: {app}\movies\flash

; movies\flash
Source: movies\flash\sources\hacker2.fla; DestDir: {app}\movies\flash\sources
Source: movies\flash\sources\this_is_aestesis.fla; DestDir: {app}\movies\flash\sources

; ompp
Source: ompp\videoPlayer.dll; DestDir: {app}\ompp
Source: ompp\videoPlayer.xml; DestDir: {app}\ompp
Source: ompp\QTplayer\back.png; DestDir: {app}\ompp\QTplayer
Source: ompp\QTplayer\button.png; DestDir: {app}\ompp\QTplayer
Source: ompp\QTplayer\front.png; DestDir: {app}\ompp\QTplayer
Source: ompp\QTplayer\knob32.png; DestDir: {app}\ompp\QTplayer
Source: ompp\QTplayer\knob24.png; DestDir: {app}\ompp\QTplayer
Source: ompp\QTplayer\play.png; DestDir: {app}\ompp\QTplayer
Source: ompp\QTplayer\revers.png; DestDir: {app}\ompp\QTplayer



; plugins
Source: plugins\98.dll; DestDir: {app}\plugins
Source: plugins\dancer.dll; DestDir: {app}\plugins
Source: plugins\flashp.dll; DestDir: {app}\plugins
Source: plugins\zool.dll; DestDir: {app}\plugins

; plugins\98
Source: plugins\98\ae_fractOfire0smooth.dll; DestDir: {app}\plugins\98
Source: plugins\98\ae_sinus03.dll; DestDir: {app}\plugins\98
Source: plugins\98\blurzoom.dll; DestDir: {app}\plugins\98
Source: plugins\98\Cubee.dll; DestDir: {app}\plugins\98
Source: plugins\98\fire24b.dll; DestDir: {app}\plugins\98
Source: plugins\98\fractozoom002b.dll; DestDir: {app}\plugins\98
Source: plugins\98\LineGrid.dll; DestDir: {app}\plugins\98
Source: plugins\98\sinus.dll; DestDir: {app}\plugins\98
Source: plugins\98\SpaceCurve001.dll; DestDir: {app}\plugins\98
Source: plugins\98\teknOscillo.dll; DestDir: {app}\plugins\98

; shaders
Source: shaders\audio_black.fx; DestDir: {app}\shaders
Source: shaders\audio_white.fx; DestDir: {app}\shaders
Source: shaders\blackForce.fx; DestDir: {app}\shaders
Source: shaders\circular.fx; DestDir: {app}\shaders
Source: shaders\color_decal.fx; DestDir: {app}\shaders
Source: shaders\destroy.fx; DestDir: {app}\shaders
Source: shaders\flash.fx; DestDir: {app}\shaders
Source: shaders\gely.fx; DestDir: {app}\shaders
Source: shaders\goazoom.fx; DestDir: {app}\shaders
Source: shaders\ripple.fx; DestDir: {app}\shaders
Source: shaders\blackwhite.fx; DestDir: {app}\shaders
Source: shaders\edge.fx; DestDir: {app}\shaders
Source: shaders\emboss.fx; DestDir: {app}\shaders
Source: shaders\invers.fx; DestDir: {app}\shaders
Source: shaders\mirror.fx; DestDir: {app}\shaders
Source: shaders\crazy.fx; DestDir: {app}\shaders
Source: shaders\zygo.fx; DestDir: {app}\shaders
Source: shaders\twirl.fx; DestDir: {app}\shaders
Source: shaders\punch.fx; DestDir: {app}\shaders
Source: shaders\zygo.fx; DestDir: {app}\shaders
Source: shaders\contrast.fx; DestDir: {app}\shaders
Source: shaders\hertz.fx; DestDir: {app}\shaders
Source: shaders\audiomono.fx; DestDir: {app}\shaders
Source: shaders\audiochrome.fx; DestDir: {app}\shaders
Source: shaders\levels.fx; DestDir: {app}\shaders


; wipes
Source: wipes\aest\slice01.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice02.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice03.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice04.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice05.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice06.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice07.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice08.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice09.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice10.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice11.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice12.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice13.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice14.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice15.png; DestDir: {app}\wipes\aest
Source: wipes\aest\slice16.png; DestDir: {app}\wipes\aest
Source: wipes\arts01\1.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\10.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\11.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\12.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\13.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\14.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\15.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\16.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\2.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\3.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\4.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\5.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\6.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\7.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\8.png; DestDir: {app}\wipes\arts01
Source: wipes\arts01\9.png; DestDir: {app}\wipes\arts01
Source: wipes\arts02\1.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\10.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\11.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\12.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\13.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\14.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\15.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\16.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\2.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\3.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\4.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\5.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\6.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\7.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\8.png; DestDir: {app}\wipes\arts02
Source: wipes\arts02\9.png; DestDir: {app}\wipes\arts02
Source: wipes\arts03\17.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\18.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\19.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\20.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\21.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\22.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\23.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\24.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\25.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\26.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\27.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\28.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\29.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\30.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\31.png; DestDir: {app}\wipes\arts03
Source: wipes\arts03\32.png; DestDir: {app}\wipes\arts03
Source: wipes\arts04\33.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\34.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\35.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\36.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\37.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\38.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\39.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\40.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\41.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\42.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\43.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\44.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\45.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\46.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\47.png; DestDir: {app}\wipes\arts04
Source: wipes\arts04\48.png; DestDir: {app}\wipes\arts04
Source: wipes\arts05\49.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\50.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\51.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\52.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\53.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\54.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\55.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\56.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\57.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\58.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\59.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\60.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\61.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\62.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\63.png; DestDir: {app}\wipes\arts05
Source: wipes\arts05\64.png; DestDir: {app}\wipes\arts05
Source: wipes\arts06\65.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\66.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\67.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\68.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\69.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\70.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\71.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\72.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\73.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\74.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\75.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\76.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\77.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\78.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\79.png; DestDir: {app}\wipes\arts06
Source: wipes\arts06\80.png; DestDir: {app}\wipes\arts06
Source: wipes\arts07\81.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\82.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\83.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\84.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\85.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\86.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\87.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\88.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\89.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\90.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\91.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\92.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\93.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\94.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\95.png; DestDir: {app}\wipes\arts07
Source: wipes\arts07\96.png; DestDir: {app}\wipes\arts07
Source: wipes\arts08\100.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\101.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\102.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\103.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\104.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\105.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\106.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\107.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\108.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\109.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\110.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\111.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\112.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\97.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\98.png; DestDir: {app}\wipes\arts08
Source: wipes\arts08\99.png; DestDir: {app}\wipes\arts08
Source: wipes\arts09\113.png; DestDir: {app}\wipes\arts09
Source: wipes\arts09\114.png; DestDir: {app}\wipes\arts09
Source: wipes\arts09\115.png; DestDir: {app}\wipes\arts09
Source: wipes\arts09\116.png; DestDir: {app}\wipes\arts09
Source: wipes\arts09\117.png; DestDir: {app}\wipes\arts09
Source: wipes\arts09\118.png; DestDir: {app}\wipes\arts09
Source: wipes\arts09\119.png; DestDir: {app}\wipes\arts09
Source: wipes\arts09\120.png; DestDir: {app}\wipes\arts09
Source: wipes\arts09\121.png; DestDir: {app}\wipes\arts09
Source: wipes\basic01\122.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\123.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\124.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\125.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\126.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\127.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\128.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\129.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\130.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\131.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\132.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\133.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\134.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\135.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\136.png; DestDir: {app}\wipes\basic01
Source: wipes\basic01\137.png; DestDir: {app}\wipes\basic01
Source: wipes\basic02\138.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\139.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\140.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\141.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\142.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\143.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\144.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\145.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\146.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\147.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\148.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\149.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\150.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\151.png; DestDir: {app}\wipes\basic02
Source: wipes\basic02\152.png; DestDir: {app}\wipes\basic02
Source: wipes\basic03\153.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\154.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\155.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\156.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\157.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\158.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\159.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\160.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\161.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\162.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\163.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\164.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\165.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\166.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\167.png; DestDir: {app}\wipes\basic03
Source: wipes\basic03\168.png; DestDir: {app}\wipes\basic03
Source: wipes\light01\169.png; DestDir: {app}\wipes\light01
Source: wipes\light01\170.png; DestDir: {app}\wipes\light01
Source: wipes\light01\171.png; DestDir: {app}\wipes\light01
Source: wipes\light01\172.png; DestDir: {app}\wipes\light01
Source: wipes\light01\173.png; DestDir: {app}\wipes\light01
Source: wipes\light01\174.png; DestDir: {app}\wipes\light01
Source: wipes\light01\175.png; DestDir: {app}\wipes\light01
Source: wipes\light01\176.png; DestDir: {app}\wipes\light01
Source: wipes\light01\177.png; DestDir: {app}\wipes\light01
Source: wipes\light01\178.png; DestDir: {app}\wipes\light01
Source: wipes\light01\179.png; DestDir: {app}\wipes\light01
Source: wipes\light01\180.png; DestDir: {app}\wipes\light01
Source: wipes\light01\181.png; DestDir: {app}\wipes\light01
Source: wipes\light01\182.png; DestDir: {app}\wipes\light01
Source: wipes\light01\183.png; DestDir: {app}\wipes\light01
Source: wipes\light02\184.png; DestDir: {app}\wipes\light02
Source: wipes\light02\185.png; DestDir: {app}\wipes\light02
Source: wipes\light02\186.png; DestDir: {app}\wipes\light02
Source: wipes\light02\187.png; DestDir: {app}\wipes\light02
Source: wipes\light02\188.png; DestDir: {app}\wipes\light02
Source: wipes\light02\189.png; DestDir: {app}\wipes\light02
Source: wipes\light02\190.png; DestDir: {app}\wipes\light02
Source: wipes\light02\191.png; DestDir: {app}\wipes\light02
Source: wipes\light02\192.png; DestDir: {app}\wipes\light02
Source: wipes\light02\193.png; DestDir: {app}\wipes\light02
Source: wipes\light02\194.png; DestDir: {app}\wipes\light02
Source: wipes\light02\195.png; DestDir: {app}\wipes\light02
Source: wipes\light02\196.png; DestDir: {app}\wipes\light02
Source: wipes\light02\197.png; DestDir: {app}\wipes\light02
Source: wipes\light03\198.png; DestDir: {app}\wipes\light03
Source: wipes\light03\199.png; DestDir: {app}\wipes\light03
Source: wipes\light03\200.png; DestDir: {app}\wipes\light03
Source: wipes\light03\201.png; DestDir: {app}\wipes\light03
Source: wipes\light03\202.png; DestDir: {app}\wipes\light03
Source: wipes\light03\203.png; DestDir: {app}\wipes\light03
Source: wipes\light03\204.png; DestDir: {app}\wipes\light03
Source: wipes\sinus01\205.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\206.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\207.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\208.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\209.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\210.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\211.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\212.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\213.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\214.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\215.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\216.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\217.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\218.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\219.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus01\220.png; DestDir: {app}\wipes\sinus01
Source: wipes\sinus02\221.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\222.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\223.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\224.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\225.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\226.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\227.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\228.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\229.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\230.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\231.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\232.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus02\233.png; DestDir: {app}\wipes\sinus02
Source: wipes\sinus03\234.png; DestDir: {app}\wipes\sinus03
Source: wipes\sinus03\235.png; DestDir: {app}\wipes\sinus03
Source: wipes\sinus03\236.png; DestDir: {app}\wipes\sinus03
Source: wipes\sinus03\237.png; DestDir: {app}\wipes\sinus03
Source: wipes\sinus03\238.png; DestDir: {app}\wipes\sinus03
Source: wipes\sinus03\239.png; DestDir: {app}\wipes\sinus03
Source: wipes\sinus03\240.png; DestDir: {app}\wipes\sinus03
Source: wipes\sinus03\241.png; DestDir: {app}\wipes\sinus03
Source: wipes\spiral01\242.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\243.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\244.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\245.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\246.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\247.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\248.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\249.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\250.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\251.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\252.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\253.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\254.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\255.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\256.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral01\257.png; DestDir: {app}\wipes\spirale01
Source: wipes\spiral02\258.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\259.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\260.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\261.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\262.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\263.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\264.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\265.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\266.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\267.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral02\268.png; DestDir: {app}\wipes\spirale02
Source: wipes\spiral03\269.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\270.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\271.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\272.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\273.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\274.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\275.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\276.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\277.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\278.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\279.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\280.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral03\281.png; DestDir: {app}\wipes\spirale03
Source: wipes\spiral04\282.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\283.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\284.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\285.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\286.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\287.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\288.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\289.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\290.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\291.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\292.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\293.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\294.png; DestDir: {app}\wipes\spirale04
Source: wipes\spiral04\295.png; DestDir: {app}\wipes\spirale04
Source: wipes\texture01\296.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\297.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\298.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\299.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\300.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\301.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\302.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\303.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\304.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\305.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\306.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\307.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\308.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\309.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\310.png; DestDir: {app}\wipes\texture01
Source: wipes\texture01\311.png; DestDir: {app}\wipes\texture01
Source: wipes\texture02\312.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\313.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\314.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\315.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\316.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\317.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\318.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\319.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\320.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\321.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\322.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\323.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\324.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\325.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\326.png; DestDir: {app}\wipes\texture02
Source: wipes\texture02\327.png; DestDir: {app}\wipes\texture02
Source: wipes\texture03\328.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\329.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\330.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\331.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\332.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\333.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\334.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\335.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\336.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\337.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\338.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\339.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\340.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\341.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\342.png; DestDir: {app}\wipes\texture03
Source: wipes\texture03\343.png; DestDir: {app}\wipes\texture03
Source: wipes\texture04\344.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\345.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\346.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\347.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\348.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\349.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\350.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\351.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\352.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\353.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\354.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\355.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\356.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\357.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\358.png; DestDir: {app}\wipes\texture04
Source: wipes\texture04\359.png; DestDir: {app}\wipes\texture04
Source: wipes\texture05\360.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\361.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\362.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\363.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\364.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\365.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\366.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\367.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\368.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\369.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\370.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\371.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\372.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\373.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\374.png; DestDir: {app}\wipes\texture05
Source: wipes\texture05\375.png; DestDir: {app}\wipes\texture05
Source: wipes\texture06\376.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\377.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\378.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\379.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\380.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\381.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\382.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\383.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\384.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\385.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\386.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\387.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\388.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\389.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\390.png; DestDir: {app}\wipes\texture06
Source: wipes\texture06\391.png; DestDir: {app}\wipes\texture06
Source: wipes\texture07\392.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\393.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\394.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\395.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\396.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\397.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\398.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\399.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\400.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\401.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\402.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\403.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\404.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\405.png; DestDir: {app}\wipes\texture07
Source: wipes\texture07\406.png; DestDir: {app}\wipes\texture07

; wwwroot
Source: wwwroot\graphapplet.class; DestDir: {app}\wwwroot
Source: wwwroot\index.html; DestDir: {app}\wwwroot
Source: wwwroot\snap160.html; DestDir: {app}\wwwroot
Source: wwwroot\snap320.html; DestDir: {app}\wwwroot
Source: wwwroot\zygot.class; DestDir: {app}\wwwroot

; main

Source: aestesis.html; DestDir: {app}
Source: alib100.dll; DestDir: {app}
Source: audio-mix.eka; DestDir: {app}
Source: audio-dub.eka; DestDir: {app}
Source: COMobjects.dll; DestDir: {app}
Source: default.eka; DestDir: {app}
Source: elektro.dll; DestDir: {app}
Source: elektronika.exe; DestDir: {app}
Source: freeframe.html; DestDir: {app}
Source: license.txt; DestDir: {app}
Source: register.html; DestDir: {app}
Source: aestesis.html; DestDir: {app}
Source: release.txt; DestDir: {app}; Flags: isreadme
Source: sample-flash.eka; DestDir: {app}
Source: sample-full-audio.eka; DestDir: {app}
Source: sample-sequence.eka; DestDir: {app}
Source: sample-simple.eka; DestDir: {app}
Source: sample-vapor.eka; DestDir: {app}
Source: video-in.eka; DestDir: {app}
Source: www.ini; DestDir: {app}
Source: elektronika.ax; DestDir: {app}; Flags: regserver
Source: iconv.dll; DestDir: {app}
Source: libxml2.dll; DestDir: {app}
Source: zlib1.dll; DestDir: {app}

; 3rd party installer

;Source: QuickTimeInstaller.exe; DestDir: {tmp}
;Source: klcodec266b.exe; Destdir: {tmp}
;Source: quicktimealt168.exe; Destdir: {tmp}
;Source: Microsoft_VC80_MFC_x86.msi; DestDir: {tmp}
;Source: Microsoft_VC80_ATL_x86.msi; DestDir: {tmp}
;Source: policy_8_0_Microsoft_VC80_MFC_x86.msi; DestDir: {tmp}
;Source: policy_8_0_Microsoft_VC80_ATL_x86.msi; DestDir: {tmp}
;Source: install_flash_player_8.msi; DestDir: {tmp}

[Icons]
Name: {userdesktop}\Elektronika Live; Filename: {app}\elektronika.exe

Name: {group}\Elektronika Live; Filename: {app}\elektronika.exe
Name: {group}\aestesis; Filename: {app}\aestesis.html
Name: {group}\Register; Filename: {app}\register.html
Name: {group}\Help; Filename: {app}\help\elektronika.chm
Name: {group}\movies; Filename: {app}\movies; Flags: foldershortcut
Name: {group}\extras; Filename: {app}\extras; Flags: foldershortcut
Name: {group}\uninstall; Filename: {uninstallexe}
Name: {group}\FreeFrame; Filename: {app}\freeframe.html



[Run]
;Filename: {tmp}\klcodec266b.exe; WorkingDir: {tmp}
;Filename: {tmp}\quicktimealt168.exe; WorkingDir: {tmp}
;Filename: {tmp}\QuickTimeInstaller.exe; WorkingDir: {tmp}; Parameters: /s /v /qn
;Filename: msiexec.exe; Parameters: "/quiet /i ""{tmp}\Microsoft_VC80_MFC_x86.msi"""
;Filename: msiexec.exe; Parameters: "/quiet /i ""{tmp}\Microsoft_VC80_ATL_x86.msi"""
;Filename: msiexec.exe; Parameters: "/quiet /i ""{tmp}\policy_8_0_Microsoft_VC80_MFC_x86.msi"""
;Filename: msiexec.exe; Parameters: "/quiet /i ""{tmp}\policy_8_0_Microsoft_VC80_ATL_x86.msi"""
;Filename: msiexec.exe; Parameters: "/quiet /i ""{tmp}\install_flash_player_8.msi"""

