EXTERN g_winmmProcs:QWORD
.code
proxy_CloseDriver PROC
    jmp qword ptr [g_winmmProcs + 0]
proxy_CloseDriver ENDP
proxy_DefDriverProc PROC
    jmp qword ptr [g_winmmProcs + 8]
proxy_DefDriverProc ENDP
proxy_DriverCallback PROC
    jmp qword ptr [g_winmmProcs + 16]
proxy_DriverCallback ENDP
proxy_DrvGetModuleHandle PROC
    jmp qword ptr [g_winmmProcs + 24]
proxy_DrvGetModuleHandle ENDP
proxy_GetDriverModuleHandle PROC
    jmp qword ptr [g_winmmProcs + 32]
proxy_GetDriverModuleHandle ENDP
proxy_OpenDriver PROC
    jmp qword ptr [g_winmmProcs + 40]
proxy_OpenDriver ENDP
proxy_PlaySound PROC
    jmp qword ptr [g_winmmProcs + 48]
proxy_PlaySound ENDP
proxy_PlaySoundA PROC
    jmp qword ptr [g_winmmProcs + 56]
proxy_PlaySoundA ENDP
proxy_PlaySoundW PROC
    jmp qword ptr [g_winmmProcs + 64]
proxy_PlaySoundW ENDP
proxy_SendDriverMessage PROC
    jmp qword ptr [g_winmmProcs + 72]
proxy_SendDriverMessage ENDP
proxy_WOWAppExit PROC
    jmp qword ptr [g_winmmProcs + 80]
proxy_WOWAppExit ENDP
proxy_auxGetDevCapsA PROC
    jmp qword ptr [g_winmmProcs + 88]
proxy_auxGetDevCapsA ENDP
proxy_auxGetDevCapsW PROC
    jmp qword ptr [g_winmmProcs + 96]
proxy_auxGetDevCapsW ENDP
proxy_auxGetNumDevs PROC
    jmp qword ptr [g_winmmProcs + 104]
proxy_auxGetNumDevs ENDP
proxy_auxGetVolume PROC
    jmp qword ptr [g_winmmProcs + 112]
proxy_auxGetVolume ENDP
proxy_auxOutMessage PROC
    jmp qword ptr [g_winmmProcs + 120]
proxy_auxOutMessage ENDP
proxy_auxSetVolume PROC
    jmp qword ptr [g_winmmProcs + 128]
proxy_auxSetVolume ENDP
proxy_joyConfigChanged PROC
    jmp qword ptr [g_winmmProcs + 136]
proxy_joyConfigChanged ENDP
proxy_joyGetDevCapsA PROC
    jmp qword ptr [g_winmmProcs + 144]
proxy_joyGetDevCapsA ENDP
proxy_joyGetDevCapsW PROC
    jmp qword ptr [g_winmmProcs + 152]
proxy_joyGetDevCapsW ENDP
proxy_joyGetNumDevs PROC
    jmp qword ptr [g_winmmProcs + 160]
proxy_joyGetNumDevs ENDP
proxy_joyGetPos PROC
    jmp qword ptr [g_winmmProcs + 168]
proxy_joyGetPos ENDP
proxy_joyGetPosEx PROC
    jmp qword ptr [g_winmmProcs + 176]
proxy_joyGetPosEx ENDP
proxy_joyGetThreshold PROC
    jmp qword ptr [g_winmmProcs + 184]
proxy_joyGetThreshold ENDP
proxy_joyReleaseCapture PROC
    jmp qword ptr [g_winmmProcs + 192]
proxy_joyReleaseCapture ENDP
proxy_joySetCapture PROC
    jmp qword ptr [g_winmmProcs + 200]
proxy_joySetCapture ENDP
proxy_joySetThreshold PROC
    jmp qword ptr [g_winmmProcs + 208]
proxy_joySetThreshold ENDP
proxy_mciDriverNotify PROC
    jmp qword ptr [g_winmmProcs + 216]
proxy_mciDriverNotify ENDP
proxy_mciDriverYield PROC
    jmp qword ptr [g_winmmProcs + 224]
proxy_mciDriverYield ENDP
proxy_mciExecute PROC
    jmp qword ptr [g_winmmProcs + 232]
proxy_mciExecute ENDP
proxy_mciFreeCommandResource PROC
    jmp qword ptr [g_winmmProcs + 240]
proxy_mciFreeCommandResource ENDP
proxy_mciGetCreatorTask PROC
    jmp qword ptr [g_winmmProcs + 248]
proxy_mciGetCreatorTask ENDP
proxy_mciGetDeviceIDA PROC
    jmp qword ptr [g_winmmProcs + 256]
proxy_mciGetDeviceIDA ENDP
proxy_mciGetDeviceIDFromElementIDA PROC
    jmp qword ptr [g_winmmProcs + 264]
proxy_mciGetDeviceIDFromElementIDA ENDP
proxy_mciGetDeviceIDFromElementIDW PROC
    jmp qword ptr [g_winmmProcs + 272]
proxy_mciGetDeviceIDFromElementIDW ENDP
proxy_mciGetDeviceIDW PROC
    jmp qword ptr [g_winmmProcs + 280]
proxy_mciGetDeviceIDW ENDP
proxy_mciGetDriverData PROC
    jmp qword ptr [g_winmmProcs + 288]
proxy_mciGetDriverData ENDP
proxy_mciGetErrorStringA PROC
    jmp qword ptr [g_winmmProcs + 296]
proxy_mciGetErrorStringA ENDP
proxy_mciGetErrorStringW PROC
    jmp qword ptr [g_winmmProcs + 304]
proxy_mciGetErrorStringW ENDP
proxy_mciGetYieldProc PROC
    jmp qword ptr [g_winmmProcs + 312]
proxy_mciGetYieldProc ENDP
proxy_mciLoadCommandResource PROC
    jmp qword ptr [g_winmmProcs + 320]
proxy_mciLoadCommandResource ENDP
proxy_mciSendCommandA PROC
    jmp qword ptr [g_winmmProcs + 328]
proxy_mciSendCommandA ENDP
proxy_mciSendCommandW PROC
    jmp qword ptr [g_winmmProcs + 336]
proxy_mciSendCommandW ENDP
proxy_mciSendStringA PROC
    jmp qword ptr [g_winmmProcs + 344]
proxy_mciSendStringA ENDP
proxy_mciSendStringW PROC
    jmp qword ptr [g_winmmProcs + 352]
proxy_mciSendStringW ENDP
proxy_mciSetDriverData PROC
    jmp qword ptr [g_winmmProcs + 360]
proxy_mciSetDriverData ENDP
proxy_mciSetYieldProc PROC
    jmp qword ptr [g_winmmProcs + 368]
proxy_mciSetYieldProc ENDP
proxy_midiConnect PROC
    jmp qword ptr [g_winmmProcs + 376]
proxy_midiConnect ENDP
proxy_midiDisconnect PROC
    jmp qword ptr [g_winmmProcs + 384]
proxy_midiDisconnect ENDP
proxy_midiInAddBuffer PROC
    jmp qword ptr [g_winmmProcs + 392]
proxy_midiInAddBuffer ENDP
proxy_midiInClose PROC
    jmp qword ptr [g_winmmProcs + 400]
proxy_midiInClose ENDP
proxy_midiInGetDevCapsA PROC
    jmp qword ptr [g_winmmProcs + 408]
proxy_midiInGetDevCapsA ENDP
proxy_midiInGetDevCapsW PROC
    jmp qword ptr [g_winmmProcs + 416]
proxy_midiInGetDevCapsW ENDP
proxy_midiInGetErrorTextA PROC
    jmp qword ptr [g_winmmProcs + 424]
proxy_midiInGetErrorTextA ENDP
proxy_midiInGetErrorTextW PROC
    jmp qword ptr [g_winmmProcs + 432]
proxy_midiInGetErrorTextW ENDP
proxy_midiInGetID PROC
    jmp qword ptr [g_winmmProcs + 440]
proxy_midiInGetID ENDP
proxy_midiInGetNumDevs PROC
    jmp qword ptr [g_winmmProcs + 448]
proxy_midiInGetNumDevs ENDP
proxy_midiInMessage PROC
    jmp qword ptr [g_winmmProcs + 456]
proxy_midiInMessage ENDP
proxy_midiInOpen PROC
    jmp qword ptr [g_winmmProcs + 464]
proxy_midiInOpen ENDP
proxy_midiInPrepareHeader PROC
    jmp qword ptr [g_winmmProcs + 472]
proxy_midiInPrepareHeader ENDP
proxy_midiInReset PROC
    jmp qword ptr [g_winmmProcs + 480]
proxy_midiInReset ENDP
proxy_midiInStart PROC
    jmp qword ptr [g_winmmProcs + 488]
proxy_midiInStart ENDP
proxy_midiInStop PROC
    jmp qword ptr [g_winmmProcs + 496]
proxy_midiInStop ENDP
proxy_midiInUnprepareHeader PROC
    jmp qword ptr [g_winmmProcs + 504]
proxy_midiInUnprepareHeader ENDP
proxy_midiOutCacheDrumPatches PROC
    jmp qword ptr [g_winmmProcs + 512]
proxy_midiOutCacheDrumPatches ENDP
proxy_midiOutCachePatches PROC
    jmp qword ptr [g_winmmProcs + 520]
proxy_midiOutCachePatches ENDP
proxy_midiOutClose PROC
    jmp qword ptr [g_winmmProcs + 528]
proxy_midiOutClose ENDP
proxy_midiOutGetDevCapsA PROC
    jmp qword ptr [g_winmmProcs + 536]
proxy_midiOutGetDevCapsA ENDP
proxy_midiOutGetDevCapsW PROC
    jmp qword ptr [g_winmmProcs + 544]
proxy_midiOutGetDevCapsW ENDP
proxy_midiOutGetErrorTextA PROC
    jmp qword ptr [g_winmmProcs + 552]
proxy_midiOutGetErrorTextA ENDP
proxy_midiOutGetErrorTextW PROC
    jmp qword ptr [g_winmmProcs + 560]
proxy_midiOutGetErrorTextW ENDP
proxy_midiOutGetID PROC
    jmp qword ptr [g_winmmProcs + 568]
proxy_midiOutGetID ENDP
proxy_midiOutGetNumDevs PROC
    jmp qword ptr [g_winmmProcs + 576]
proxy_midiOutGetNumDevs ENDP
proxy_midiOutGetVolume PROC
    jmp qword ptr [g_winmmProcs + 584]
proxy_midiOutGetVolume ENDP
proxy_midiOutLongMsg PROC
    jmp qword ptr [g_winmmProcs + 592]
proxy_midiOutLongMsg ENDP
proxy_midiOutMessage PROC
    jmp qword ptr [g_winmmProcs + 600]
proxy_midiOutMessage ENDP
proxy_midiOutOpen PROC
    jmp qword ptr [g_winmmProcs + 608]
proxy_midiOutOpen ENDP
proxy_midiOutPrepareHeader PROC
    jmp qword ptr [g_winmmProcs + 616]
proxy_midiOutPrepareHeader ENDP
proxy_midiOutReset PROC
    jmp qword ptr [g_winmmProcs + 624]
proxy_midiOutReset ENDP
proxy_midiOutSetVolume PROC
    jmp qword ptr [g_winmmProcs + 632]
proxy_midiOutSetVolume ENDP
proxy_midiOutShortMsg PROC
    jmp qword ptr [g_winmmProcs + 640]
proxy_midiOutShortMsg ENDP
proxy_midiOutUnprepareHeader PROC
    jmp qword ptr [g_winmmProcs + 648]
proxy_midiOutUnprepareHeader ENDP
proxy_midiStreamClose PROC
    jmp qword ptr [g_winmmProcs + 656]
proxy_midiStreamClose ENDP
proxy_midiStreamOpen PROC
    jmp qword ptr [g_winmmProcs + 664]
proxy_midiStreamOpen ENDP
proxy_midiStreamOut PROC
    jmp qword ptr [g_winmmProcs + 672]
proxy_midiStreamOut ENDP
proxy_midiStreamPause PROC
    jmp qword ptr [g_winmmProcs + 680]
proxy_midiStreamPause ENDP
proxy_midiStreamPosition PROC
    jmp qword ptr [g_winmmProcs + 688]
proxy_midiStreamPosition ENDP
proxy_midiStreamProperty PROC
    jmp qword ptr [g_winmmProcs + 696]
proxy_midiStreamProperty ENDP
proxy_midiStreamRestart PROC
    jmp qword ptr [g_winmmProcs + 704]
proxy_midiStreamRestart ENDP
proxy_midiStreamStop PROC
    jmp qword ptr [g_winmmProcs + 712]
proxy_midiStreamStop ENDP
proxy_mixerClose PROC
    jmp qword ptr [g_winmmProcs + 720]
proxy_mixerClose ENDP
proxy_mixerGetControlDetailsA PROC
    jmp qword ptr [g_winmmProcs + 728]
proxy_mixerGetControlDetailsA ENDP
proxy_mixerGetControlDetailsW PROC
    jmp qword ptr [g_winmmProcs + 736]
proxy_mixerGetControlDetailsW ENDP
proxy_mixerGetDevCapsA PROC
    jmp qword ptr [g_winmmProcs + 744]
proxy_mixerGetDevCapsA ENDP
proxy_mixerGetDevCapsW PROC
    jmp qword ptr [g_winmmProcs + 752]
proxy_mixerGetDevCapsW ENDP
proxy_mixerGetID PROC
    jmp qword ptr [g_winmmProcs + 760]
proxy_mixerGetID ENDP
proxy_mixerGetLineControlsA PROC
    jmp qword ptr [g_winmmProcs + 768]
proxy_mixerGetLineControlsA ENDP
proxy_mixerGetLineControlsW PROC
    jmp qword ptr [g_winmmProcs + 776]
proxy_mixerGetLineControlsW ENDP
proxy_mixerGetLineInfoA PROC
    jmp qword ptr [g_winmmProcs + 784]
proxy_mixerGetLineInfoA ENDP
proxy_mixerGetLineInfoW PROC
    jmp qword ptr [g_winmmProcs + 792]
proxy_mixerGetLineInfoW ENDP
proxy_mixerGetNumDevs PROC
    jmp qword ptr [g_winmmProcs + 800]
proxy_mixerGetNumDevs ENDP
proxy_mixerMessage PROC
    jmp qword ptr [g_winmmProcs + 808]
proxy_mixerMessage ENDP
proxy_mixerOpen PROC
    jmp qword ptr [g_winmmProcs + 816]
proxy_mixerOpen ENDP
proxy_mixerSetControlDetails PROC
    jmp qword ptr [g_winmmProcs + 824]
proxy_mixerSetControlDetails ENDP
proxy_mmDrvInstall PROC
    jmp qword ptr [g_winmmProcs + 832]
proxy_mmDrvInstall ENDP
proxy_mmGetCurrentTask PROC
    jmp qword ptr [g_winmmProcs + 840]
proxy_mmGetCurrentTask ENDP
proxy_mmTaskBlock PROC
    jmp qword ptr [g_winmmProcs + 848]
proxy_mmTaskBlock ENDP
proxy_mmTaskCreate PROC
    jmp qword ptr [g_winmmProcs + 856]
proxy_mmTaskCreate ENDP
proxy_mmTaskSignal PROC
    jmp qword ptr [g_winmmProcs + 864]
proxy_mmTaskSignal ENDP
proxy_mmTaskYield PROC
    jmp qword ptr [g_winmmProcs + 872]
proxy_mmTaskYield ENDP
proxy_mmioAdvance PROC
    jmp qword ptr [g_winmmProcs + 880]
proxy_mmioAdvance ENDP
proxy_mmioAscend PROC
    jmp qword ptr [g_winmmProcs + 888]
proxy_mmioAscend ENDP
proxy_mmioClose PROC
    jmp qword ptr [g_winmmProcs + 896]
proxy_mmioClose ENDP
proxy_mmioCreateChunk PROC
    jmp qword ptr [g_winmmProcs + 904]
proxy_mmioCreateChunk ENDP
proxy_mmioDescend PROC
    jmp qword ptr [g_winmmProcs + 912]
proxy_mmioDescend ENDP
proxy_mmioFlush PROC
    jmp qword ptr [g_winmmProcs + 920]
proxy_mmioFlush ENDP
proxy_mmioGetInfo PROC
    jmp qword ptr [g_winmmProcs + 928]
proxy_mmioGetInfo ENDP
proxy_mmioInstallIOProcA PROC
    jmp qword ptr [g_winmmProcs + 936]
proxy_mmioInstallIOProcA ENDP
proxy_mmioInstallIOProcW PROC
    jmp qword ptr [g_winmmProcs + 944]
proxy_mmioInstallIOProcW ENDP
proxy_mmioOpenA PROC
    jmp qword ptr [g_winmmProcs + 952]
proxy_mmioOpenA ENDP
proxy_mmioOpenW PROC
    jmp qword ptr [g_winmmProcs + 960]
proxy_mmioOpenW ENDP
proxy_mmioRead PROC
    jmp qword ptr [g_winmmProcs + 968]
proxy_mmioRead ENDP
proxy_mmioRenameA PROC
    jmp qword ptr [g_winmmProcs + 976]
proxy_mmioRenameA ENDP
proxy_mmioRenameW PROC
    jmp qword ptr [g_winmmProcs + 984]
proxy_mmioRenameW ENDP
proxy_mmioSeek PROC
    jmp qword ptr [g_winmmProcs + 992]
proxy_mmioSeek ENDP
proxy_mmioSendMessage PROC
    jmp qword ptr [g_winmmProcs + 1000]
proxy_mmioSendMessage ENDP
proxy_mmioSetBuffer PROC
    jmp qword ptr [g_winmmProcs + 1008]
proxy_mmioSetBuffer ENDP
proxy_mmioSetInfo PROC
    jmp qword ptr [g_winmmProcs + 1016]
proxy_mmioSetInfo ENDP
proxy_mmioStringToFOURCCA PROC
    jmp qword ptr [g_winmmProcs + 1024]
proxy_mmioStringToFOURCCA ENDP
proxy_mmioStringToFOURCCW PROC
    jmp qword ptr [g_winmmProcs + 1032]
proxy_mmioStringToFOURCCW ENDP
proxy_mmioWrite PROC
    jmp qword ptr [g_winmmProcs + 1040]
proxy_mmioWrite ENDP
proxy_mmsystemGetVersion PROC
    jmp qword ptr [g_winmmProcs + 1048]
proxy_mmsystemGetVersion ENDP
proxy_sndPlaySoundA PROC
    jmp qword ptr [g_winmmProcs + 1056]
proxy_sndPlaySoundA ENDP
proxy_sndPlaySoundW PROC
    jmp qword ptr [g_winmmProcs + 1064]
proxy_sndPlaySoundW ENDP
proxy_timeBeginPeriod PROC
    jmp qword ptr [g_winmmProcs + 1072]
proxy_timeBeginPeriod ENDP
proxy_timeEndPeriod PROC
    jmp qword ptr [g_winmmProcs + 1080]
proxy_timeEndPeriod ENDP
proxy_timeGetDevCaps PROC
    jmp qword ptr [g_winmmProcs + 1088]
proxy_timeGetDevCaps ENDP
proxy_timeGetSystemTime PROC
    jmp qword ptr [g_winmmProcs + 1096]
proxy_timeGetSystemTime ENDP
proxy_timeGetTime PROC
    jmp qword ptr [g_winmmProcs + 1104]
proxy_timeGetTime ENDP
proxy_timeKillEvent PROC
    jmp qword ptr [g_winmmProcs + 1112]
proxy_timeKillEvent ENDP
proxy_timeSetEvent PROC
    jmp qword ptr [g_winmmProcs + 1120]
proxy_timeSetEvent ENDP
proxy_waveInAddBuffer PROC
    jmp qword ptr [g_winmmProcs + 1128]
proxy_waveInAddBuffer ENDP
proxy_waveInClose PROC
    jmp qword ptr [g_winmmProcs + 1136]
proxy_waveInClose ENDP
proxy_waveInGetDevCapsA PROC
    jmp qword ptr [g_winmmProcs + 1144]
proxy_waveInGetDevCapsA ENDP
proxy_waveInGetDevCapsW PROC
    jmp qword ptr [g_winmmProcs + 1152]
proxy_waveInGetDevCapsW ENDP
proxy_waveInGetErrorTextA PROC
    jmp qword ptr [g_winmmProcs + 1160]
proxy_waveInGetErrorTextA ENDP
proxy_waveInGetErrorTextW PROC
    jmp qword ptr [g_winmmProcs + 1168]
proxy_waveInGetErrorTextW ENDP
proxy_waveInGetID PROC
    jmp qword ptr [g_winmmProcs + 1176]
proxy_waveInGetID ENDP
proxy_waveInGetNumDevs PROC
    jmp qword ptr [g_winmmProcs + 1184]
proxy_waveInGetNumDevs ENDP
proxy_waveInGetPosition PROC
    jmp qword ptr [g_winmmProcs + 1192]
proxy_waveInGetPosition ENDP
proxy_waveInMessage PROC
    jmp qword ptr [g_winmmProcs + 1200]
proxy_waveInMessage ENDP
proxy_waveInOpen PROC
    jmp qword ptr [g_winmmProcs + 1208]
proxy_waveInOpen ENDP
proxy_waveInPrepareHeader PROC
    jmp qword ptr [g_winmmProcs + 1216]
proxy_waveInPrepareHeader ENDP
proxy_waveInReset PROC
    jmp qword ptr [g_winmmProcs + 1224]
proxy_waveInReset ENDP
proxy_waveInStart PROC
    jmp qword ptr [g_winmmProcs + 1232]
proxy_waveInStart ENDP
proxy_waveInStop PROC
    jmp qword ptr [g_winmmProcs + 1240]
proxy_waveInStop ENDP
proxy_waveInUnprepareHeader PROC
    jmp qword ptr [g_winmmProcs + 1248]
proxy_waveInUnprepareHeader ENDP
proxy_waveOutBreakLoop PROC
    jmp qword ptr [g_winmmProcs + 1256]
proxy_waveOutBreakLoop ENDP
proxy_waveOutClose PROC
    jmp qword ptr [g_winmmProcs + 1264]
proxy_waveOutClose ENDP
proxy_waveOutGetDevCapsA PROC
    jmp qword ptr [g_winmmProcs + 1272]
proxy_waveOutGetDevCapsA ENDP
proxy_waveOutGetDevCapsW PROC
    jmp qword ptr [g_winmmProcs + 1280]
proxy_waveOutGetDevCapsW ENDP
proxy_waveOutGetErrorTextA PROC
    jmp qword ptr [g_winmmProcs + 1288]
proxy_waveOutGetErrorTextA ENDP
proxy_waveOutGetErrorTextW PROC
    jmp qword ptr [g_winmmProcs + 1296]
proxy_waveOutGetErrorTextW ENDP
proxy_waveOutGetID PROC
    jmp qword ptr [g_winmmProcs + 1304]
proxy_waveOutGetID ENDP
proxy_waveOutGetNumDevs PROC
    jmp qword ptr [g_winmmProcs + 1312]
proxy_waveOutGetNumDevs ENDP
proxy_waveOutGetPitch PROC
    jmp qword ptr [g_winmmProcs + 1320]
proxy_waveOutGetPitch ENDP
proxy_waveOutGetPlaybackRate PROC
    jmp qword ptr [g_winmmProcs + 1328]
proxy_waveOutGetPlaybackRate ENDP
proxy_waveOutGetPosition PROC
    jmp qword ptr [g_winmmProcs + 1336]
proxy_waveOutGetPosition ENDP
proxy_waveOutGetVolume PROC
    jmp qword ptr [g_winmmProcs + 1344]
proxy_waveOutGetVolume ENDP
proxy_waveOutMessage PROC
    jmp qword ptr [g_winmmProcs + 1352]
proxy_waveOutMessage ENDP
proxy_waveOutOpen PROC
    jmp qword ptr [g_winmmProcs + 1360]
proxy_waveOutOpen ENDP
proxy_waveOutPause PROC
    jmp qword ptr [g_winmmProcs + 1368]
proxy_waveOutPause ENDP
proxy_waveOutPrepareHeader PROC
    jmp qword ptr [g_winmmProcs + 1376]
proxy_waveOutPrepareHeader ENDP
proxy_waveOutReset PROC
    jmp qword ptr [g_winmmProcs + 1384]
proxy_waveOutReset ENDP
proxy_waveOutRestart PROC
    jmp qword ptr [g_winmmProcs + 1392]
proxy_waveOutRestart ENDP
proxy_waveOutSetPitch PROC
    jmp qword ptr [g_winmmProcs + 1400]
proxy_waveOutSetPitch ENDP
proxy_waveOutSetPlaybackRate PROC
    jmp qword ptr [g_winmmProcs + 1408]
proxy_waveOutSetPlaybackRate ENDP
proxy_waveOutSetVolume PROC
    jmp qword ptr [g_winmmProcs + 1416]
proxy_waveOutSetVolume ENDP
proxy_waveOutUnprepareHeader PROC
    jmp qword ptr [g_winmmProcs + 1424]
proxy_waveOutUnprepareHeader ENDP
proxy_waveOutWrite PROC
    jmp qword ptr [g_winmmProcs + 1432]
proxy_waveOutWrite ENDP
END
