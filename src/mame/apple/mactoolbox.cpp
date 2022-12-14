// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont

#include "emu.h"
#include "mactoolbox.h"

/* *************************************************************************
 * Trap Tracing
 *
 * This is debug code that will output diagnostics regarding OS traps called
 * *************************************************************************/

static const char *lookup_trap(uint16_t opcode)
{
	static const struct
	{
		uint16_t trap;
		const char *name;
	} traps[] =
	{
		{ 0xA000, "_Open" },
		{ 0xA001, "_Close" },
		{ 0xA002, "_Read" },
		{ 0xA003, "_Write" },
		{ 0xA004, "_Control" },
		{ 0xA005, "_Status" },
		{ 0xA006, "_KillIO" },
		{ 0xA007, "_GetVolInfo" },
		{ 0xA008, "_Create" },
		{ 0xA009, "_Delete" },
		{ 0xA00A, "_OpenRF" },
		{ 0xA00B, "_Rename" },
		{ 0xA00C, "_GetFileInfo" },
		{ 0xA00D, "_SetFileInfo" },
		{ 0xA00E, "_UnmountVol" },
		{ 0xA00F, "_MountVol" },
		{ 0xA010, "_Allocate" },
		{ 0xA011, "_GetEOF" },
		{ 0xA012, "_SetEOF" },
		{ 0xA013, "_FlushVol" },
		{ 0xA014, "_GetVol" },
		{ 0xA015, "_SetVol" },
		{ 0xA016, "_FInitQueue" },
		{ 0xA017, "_Eject" },
		{ 0xA018, "_GetFPos" },
		{ 0xA019, "_InitZone" },
		{ 0xA01B, "_SetZone" },
		{ 0xA01C, "_FreeMem" },
		{ 0xA01F, "_DisposePtr" },
		{ 0xA020, "_SetPtrSize" },
		{ 0xA021, "_GetPtrSize" },
		{ 0xA023, "_DisposeHandle" },
		{ 0xA024, "_SetHandleSize" },
		{ 0xA025, "_GetHandleSize" },
		{ 0xA027, "_ReallocHandle" },
		{ 0xA029, "_HLock" },
		{ 0xA02A, "_HUnlock" },
		{ 0xA02B, "_EmptyHandle" },
		{ 0xA02C, "_InitApplZone" },
		{ 0xA02D, "_SetApplLimit" },
		{ 0xA02E, "_BlockMove" },
		{ 0xA02F, "_PostEvent" },
		{ 0xA030, "_OSEventAvail" },
		{ 0xA031, "_GetOSEvent" },
		{ 0xA032, "_FlushEvents" },
		{ 0xA033, "_VInstall" },
		{ 0xA034, "_VRemove" },
		{ 0xA035, "_OffLine" },
		{ 0xA036, "_MoreMasters" },
		{ 0xA038, "_WriteParam" },
		{ 0xA039, "_ReadDateTime" },
		{ 0xA03A, "_SetDateTime" },
		{ 0xA03B, "_Delay" },
		{ 0xA03C, "_CmpString" },
		{ 0xA03D, "_DrvrInstall" },
		{ 0xA03E, "_DrvrRemove" },
		{ 0xA03F, "_InitUtil" },
		{ 0xA040, "_ResrvMem" },
		{ 0xA041, "_SetFilLock" },
		{ 0xA042, "_RstFilLock" },
		{ 0xA043, "_SetFilType" },
		{ 0xA044, "_SetFPos" },
		{ 0xA045, "_FlushFile" },
		{ 0xA047, "_SetTrapAddress" },
		{ 0xA049, "_HPurge" },
		{ 0xA04A, "_HNoPurge" },
		{ 0xA04B, "_SetGrowZone" },
		{ 0xA04C, "_CompactMem" },
		{ 0xA04D, "_PurgeMem" },
		{ 0xA04E, "_AddDrive" },
		{ 0xA04F, "_RDrvrInstall" },
		{ 0xA050, "_CompareString" },
		{ 0xA051, "_ReadXPRam" },
		{ 0xA052, "_WriteXPRam" },
		{ 0xA054, "_UprString" },
		{ 0xA055, "_StripAddress" },
		{ 0xA056, "_LowerText" },
		{ 0xA057, "_SetAppBase" },
		{ 0xA058, "_InsTime" },
		{ 0xA059, "_RmvTime" },
		{ 0xA05A, "_PrimeTime" },
		{ 0xA05B, "_PowerOff" },
		{ 0xA05C, "_MemoryDispatch" },
		{ 0xA05D, "_SwapMMUMode" },
		{ 0xA05E, "_NMInstall" },
		{ 0xA05F, "_NMRemove" },
		{ 0xA060, "_FSDispatch" },
		{ 0xA061, "_MaxBlock" },
		{ 0xA062, "_PurgeSpace" },
		{ 0xA063, "_MaxApplZone" },
		{ 0xA064, "_MoveHHi" },
		{ 0xA065, "_StackSpace" },
		{ 0xA066, "_NewEmptyHandle" },
		{ 0xA067, "_HSetRBit" },
		{ 0xA068, "_HClrRBit" },
		{ 0xA069, "_HGetState" },
		{ 0xA06A, "_HSetState" },
		{ 0xA06C, "_InitFS" },
		{ 0xA06D, "_InitEvents" },
		{ 0xA06E, "_SlotManager" },
		{ 0xA06F, "_SlotVInstall" },
		{ 0xA070, "_SlotVRemove" },
		{ 0xA071, "_AttachVBL" },
		{ 0xA072, "_DoVBLTask" },
		{ 0xA075, "_SIntInstall" },
		{ 0xA076, "_SIntRemove" },
		{ 0xA077, "_CountADBs" },
		{ 0xA078, "_GetIndADB" },
		{ 0xA079, "_GetADBInfo" },
		{ 0xA07A, "_SetADBInfo" },
		{ 0xA07B, "_ADBReInit" },
		{ 0xA07C, "_ADBOp" },
		{ 0xA07D, "_GetDefaultStartup" },
		{ 0xA07E, "_SetDefaultStartup" },
		{ 0xA07F, "_InternalWait" },
		{ 0xA080, "_GetVideoDefault" },
		{ 0xA081, "_SetVideoDefault" },
		{ 0xA082, "_DTInstall" },
		{ 0xA083, "_SetOSDefault" },
		{ 0xA084, "_GetOSDefault" },
		{ 0xA085, "_PMgrOp" },
		{ 0xA086, "_IOPInfoAccess" },
		{ 0xA087, "_IOPMsgRequest" },
		{ 0xA088, "_IOPMoveData" },
		{ 0xA089, "_SCSIAtomic" },
		{ 0xA08A, "_Sleep" },
		{ 0xA08B, "_CommToolboxDispatch" },
		{ 0xA08D, "_DebugUtil" },
		{ 0xA08F, "_DeferUserFn" },
		{ 0xA090, "_SysEnvirons" },
		{ 0xA091, "_Translate24To32" },
		{ 0xA092, "_EgretDispatch" },
		{ 0xA094, "_ServerDispatch" },
		{ 0xA09E, "_PowerMgrDispatch" },
		{ 0xA09F, "_PowerDispatch" },
		{ 0xA0A4, "_HeapDispatch" },
		{ 0xA0AC, "_FSMDispatch" },
		{ 0xA0AE, "_VADBProc" },
		{ 0xA0DD, "_PPC" },
		{ 0xA0FE, "_TEFindWord" },
		{ 0xA0FF, "_TEFindLine" },
		{ 0xA11A, "_GetZone" },
		{ 0xA11D, "_MaxMem" },
		{ 0xA11E, "_NewPtr" },
		{ 0xA122, "_NewHandle" },
		{ 0xA126, "_HandleZone" },
		{ 0xA128, "_RecoverHandle" },
		{ 0xA12F, "_PPostEvent" },
		{ 0xA146, "_GetTrapAddress" },
		{ 0xA148, "_PtrZone" },
		{ 0xA15C, "_MemoryDispatchA0Result" },
		{ 0xA162, "_PurgeSpace" },
		{ 0xA166, "_NewEmptyHandle" },
		{ 0xA193, "_Microseconds" },
		{ 0xA198, "_HWPriv" },
		{ 0xA1AD, "_Gestalt" },
		{ 0xA200, "_HOpen" },
		{ 0xA207, "_HGetVInfo" },
		{ 0xA208, "_HCreate" },
		{ 0xA209, "_HDelete" },
		{ 0xA20A, "_HOpenRF" },
		{ 0xA20B, "_HRename" },
		{ 0xA20C, "_HGetFileInfo" },
		{ 0xA20D, "_HSetFileInfo" },
		{ 0xA20E, "_HUnmountVol" },
		{ 0xA210, "_AllocContig" },
		{ 0xA214, "_HGetVol" },
		{ 0xA215, "_HSetVol" },
		{ 0xA22E, "_BlockMoveData" },
		{ 0xA23C, "_CmpStringMarks" },
		{ 0xA241, "_HSetFLock" },
		{ 0xA242, "_HRstFLock" },
		{ 0xA247, "_SetOSTrapAddress" },
		{ 0xA256, "_StripText" },
		{ 0xA260, "_HFSDispatch" },
		{ 0xA285, "_IdleUpdate" },
		{ 0xA28A, "_SlpQInstall" },
		{ 0xA31E, "_NewPtrClear" },
		{ 0xA322, "_NewHandleClear" },
		{ 0xA346, "_GetOSTrapAddress" },
		{ 0xA3AD, "_NewGestalt" },
		{ 0xA402, "_ReadAsync" },
		{ 0xA403, "_WriteAsync" },
		{ 0xA404, "_ControlAsync" },
		{ 0xA41C, "_FreeMemSys" },
		{ 0xA43C, "_CmpStringCase" },
		{ 0xA43D, "_DrvrInstallResrvMem" },
		{ 0xA440, "_ResrvMemSys" },
		{ 0xA44D, "_PurgeMemSys" },
		{ 0xA456, "_UpperText" },
		{ 0xA458, "_InsXTime" },
		{ 0xA485, "_IdleState" },
		{ 0xA48A, "_SlpQRemove" },
		{ 0xA51E, "_NewPtrSys" },
		{ 0xA522, "_NewHandleSys" },
		{ 0xA562, "_PurgeSpaceSys" },
		{ 0xA5AD, "_ReplaceGestalt" },
		{ 0xA63C, "_CmpStringCaseMarks" },
		{ 0xA647, "_SetToolBoxTrapAddress" },
		{ 0xA656, "_StripUpperText" },
		{ 0xA685, "_SerialPower" },
		{ 0xA71E, "_NewPtrSysClear" },
		{ 0xA722, "_NewHandleSysClear" },
		{ 0xA746, "_GetToolBoxTrapAddress" },
		{ 0xA7AD, "_GetGestaltProcPtr" },
		{ 0xA800, "_SoundDispatch" },
		{ 0xA801, "_SndDisposeChannel" },
		{ 0xA802, "_SndAddModifier" },
		{ 0xA803, "_SndDoCommand" },
		{ 0xA804, "_SndDoImmediate" },
		{ 0xA805, "_SndPlay" },
		{ 0xA806, "_SndControl" },
		{ 0xA807, "_SndNewChannel" },
		{ 0xA808, "_InitProcMenu" },
		{ 0xA809, "_GetControlVariant" },
		{ 0xA80A, "_GetWVariant" },
		{ 0xA80B, "_PopUpMenuSelect" },
		{ 0xA80C, "_RGetResource" },
		{ 0xA80D, "_Count1Resources" },
		{ 0xA80E, "_Get1IxResource" },
		{ 0xA80F, "_Get1IxType" },
		{ 0xA810, "_Unique1ID" },
		{ 0xA811, "_TESelView" },
		{ 0xA812, "_TEPinScroll" },
		{ 0xA813, "_TEAutoView" },
		{ 0xA814, "_SetFractEnable" },
		{ 0xA815, "_SCSIDispatch" },
		{ 0xA817, "_CopyMask" },
		{ 0xA818, "_FixATan2" },
		{ 0xA819, "_XMunger" },
		{ 0xA81A, "_HOpenResFile" },
		{ 0xA81B, "_HCreateResFile" },
		{ 0xA81C, "_Count1Types" },
		{ 0xA81D, "_InvalMenuBar" },
		{ 0xA81F, "_Get1Resource" },
		{ 0xA820, "_Get1NamedResource" },
		{ 0xA821, "_MaxSizeRsrc" },
		{ 0xA822, "_ResourceDispatch" },
		{ 0xA823, "_AliasDispatch" },
		{ 0xA824, "_HFSUtilDispatch" },
		{ 0xA825, "_MenuDispatch" },
		{ 0xA826, "_InsertMenuItem" },
		{ 0xA827, "_HideDialogItem" },
		{ 0xA828, "_ShowDialogItem" },
		{ 0xA82A, "_ComponentDispatch" },
		{ 0xA833, "_ScrnBitMap" },
		{ 0xA834, "_SetFScaleDisable" },
		{ 0xA835, "_FontMetrics" },
		{ 0xA836, "_GetMaskTable" },
		{ 0xA837, "_MeasureText" },
		{ 0xA838, "_CalcMask" },
		{ 0xA839, "_SeedFill" },
		{ 0xA83A, "_ZoomWindow" },
		{ 0xA83B, "_TrackBox" },
		{ 0xA83C, "_TEGetOffset" },
		{ 0xA83D, "_TEDispatch" },
		{ 0xA83E, "_TEStyleNew" },
		{ 0xA83F, "_Long2Fix" },
		{ 0xA840, "_Fix2Long" },
		{ 0xA841, "_Fix2Frac" },
		{ 0xA842, "_Frac2Fix" },
		{ 0xA843, "_Fix2X" },
		{ 0xA844, "_X2Fix" },
		{ 0xA845, "_Frac2X" },
		{ 0xA846, "_X2Frac" },
		{ 0xA847, "_FracCos" },
		{ 0xA848, "_FracSin" },
		{ 0xA849, "_FracSqrt" },
		{ 0xA84A, "_FracMul" },
		{ 0xA84B, "_FracDiv" },
		{ 0xA84D, "_FixDiv" },
		{ 0xA84E, "_GetItemCmd" },
		{ 0xA84F, "_SetItemCmd" },
		{ 0xA850, "_InitCursor" },
		{ 0xA851, "_SetCursor" },
		{ 0xA852, "_HideCursor" },
		{ 0xA853, "_ShowCursor" },
		{ 0xA854, "_FontDispatch" },
		{ 0xA855, "_ShieldCursor" },
		{ 0xA856, "_ObscureCursor" },
		{ 0xA857, "_SetAppBase" },
		{ 0xA858, "_BitAnd" },
		{ 0xA859, "_BitXOr" },
		{ 0xA85A, "_BitNot" },
		{ 0xA85B, "_BitOr" },
		{ 0xA85C, "_BitShift" },
		{ 0xA85D, "_BitTst" },
		{ 0xA85E, "_BitSet" },
		{ 0xA85F, "_BitClr" },
		{ 0xA860, "_WaitNextEvent" },
		{ 0xA861, "_Random" },
		{ 0xA862, "_ForeColor" },
		{ 0xA863, "_BackColor" },
		{ 0xA864, "_ColorBit" },
		{ 0xA865, "_GetPixel" },
		{ 0xA866, "_StuffHex" },
		{ 0xA867, "_LongMul" },
		{ 0xA868, "_FixMul" },
		{ 0xA869, "_FixRatio" },
		{ 0xA86A, "_HiWord" },
		{ 0xA86B, "_LoWord" },
		{ 0xA86C, "_FixRound" },
		{ 0xA86D, "_InitPort" },
		{ 0xA86E, "_InitGraf" },
		{ 0xA86F, "_OpenPort" },
		{ 0xA870, "_LocalToGlobal" },
		{ 0xA871, "_GlobalToLocal" },
		{ 0xA872, "_GrafDevice" },
		{ 0xA873, "_SetPort" },
		{ 0xA874, "_GetPort" },
		{ 0xA875, "_SetPBits" },
		{ 0xA876, "_PortSize" },
		{ 0xA877, "_MovePortTo" },
		{ 0xA878, "_SetOrigin" },
		{ 0xA879, "_SetClip" },
		{ 0xA87A, "_GetClip" },
		{ 0xA87B, "_ClipRect" },
		{ 0xA87C, "_BackPat" },
		{ 0xA87D, "_ClosePort" },
		{ 0xA87E, "_AddPt" },
		{ 0xA87F, "_SubPt" },
		{ 0xA880, "_SetPt" },
		{ 0xA881, "_EqualPt" },
		{ 0xA882, "_StdText" },
		{ 0xA883, "_DrawChar" },
		{ 0xA884, "_DrawString" },
		{ 0xA885, "_DrawText" },
		{ 0xA886, "_TextWidth" },
		{ 0xA887, "_TextFont" },
		{ 0xA888, "_TextFace" },
		{ 0xA889, "_TextMode" },
		{ 0xA88A, "_TextSize" },
		{ 0xA88B, "_GetFontInfo" },
		{ 0xA88C, "_StringWidth" },
		{ 0xA88D, "_CharWidth" },
		{ 0xA88E, "_SpaceExtra" },
		{ 0xA88F, "_OSDispatch" },
		{ 0xA890, "_StdLine" },
		{ 0xA891, "_LineTo" },
		{ 0xA892, "_Line" },
		{ 0xA893, "_MoveTo" },
		{ 0xA894, "_Move" },
		{ 0xA895, "_ShutDown" },
		{ 0xA896, "_HidePen" },
		{ 0xA897, "_ShowPen" },
		{ 0xA898, "_GetPenState" },
		{ 0xA899, "_SetPenState" },
		{ 0xA89A, "_GetPen" },
		{ 0xA89B, "_PenSize" },
		{ 0xA89C, "_PenMode" },
		{ 0xA89D, "_PenPat" },
		{ 0xA89E, "_PenNormal" },
		{ 0xA89F, "_Moof" },
		{ 0xA8A0, "_StdRect" },
		{ 0xA8A1, "_FrameRect" },
		{ 0xA8A2, "_PaintRect" },
		{ 0xA8A3, "_EraseRect" },
		{ 0xA8A4, "_InverRect" },
		{ 0xA8A5, "_FillRect" },
		{ 0xA8A6, "_EqualRect" },
		{ 0xA8A7, "_SetRect" },
		{ 0xA8A8, "_OffsetRect" },
		{ 0xA8A9, "_InsetRect" },
		{ 0xA8AA, "_SectRect" },
		{ 0xA8AB, "_UnionRect" },
		{ 0xA8AC, "_Pt2Rect" },
		{ 0xA8AD, "_PtInRect" },
		{ 0xA8AE, "_EmptyRect" },
		{ 0xA8AF, "_StdRRect" },
		{ 0xA8B0, "_FrameRoundRect" },
		{ 0xA8B1, "_PaintRoundRect" },
		{ 0xA8B2, "_EraseRoundRect" },
		{ 0xA8B3, "_InverRoundRect" },
		{ 0xA8B4, "_FillRoundRect" },
		{ 0xA8B5, "_ScriptUtil" },
		{ 0xA8B6, "_StdOval" },
		{ 0xA8B7, "_FrameOval" },
		{ 0xA8B8, "_PaintOval" },
		{ 0xA8B9, "_EraseOval" },
		{ 0xA8BA, "_InvertOval" },
		{ 0xA8BB, "_FillOval" },
		{ 0xA8BC, "_SlopeFromAngle" },
		{ 0xA8BD, "_StdArc" },
		{ 0xA8BE, "_FrameArc" },
		{ 0xA8BF, "_PaintArc" },
		{ 0xA8C0, "_EraseArc" },
		{ 0xA8C1, "_InvertArc" },
		{ 0xA8C2, "_FillArc" },
		{ 0xA8C3, "_PtToAngle" },
		{ 0xA8C4, "_AngleFromSlope" },
		{ 0xA8C5, "_StdPoly" },
		{ 0xA8C6, "_FramePoly" },
		{ 0xA8C7, "_PaintPoly" },
		{ 0xA8C8, "_ErasePoly" },
		{ 0xA8C9, "_InvertPoly" },
		{ 0xA8CA, "_FillPoly" },
		{ 0xA8CB, "_OpenPoly" },
		{ 0xA8CC, "_ClosePoly" },
		{ 0xA8CD, "_KillPoly" },
		{ 0xA8CE, "_OffsetPoly" },
		{ 0xA8CF, "_PackBits" },
		{ 0xA8D0, "_UnpackBits" },
		{ 0xA8D1, "_StdRgn" },
		{ 0xA8D2, "_FrameRgn" },
		{ 0xA8D3, "_PaintRgn" },
		{ 0xA8D4, "_EraseRgn" },
		{ 0xA8D5, "_InverRgn" },
		{ 0xA8D6, "_FillRgn" },
		{ 0xA8D7, "_BitMapToRegion" },
		{ 0xA8D8, "_NewRgn" },
		{ 0xA8D9, "_DisposeRgn" },
		{ 0xA8DA, "_OpenRgn" },
		{ 0xA8DB, "_CloseRgn" },
		{ 0xA8DC, "_CopyRgn" },
		{ 0xA8DD, "_SetEmptyRgn" },
		{ 0xA8DE, "_SetRecRgn" },
		{ 0xA8DF, "_RectRgn" },
		{ 0xA8E0, "_OffsetRgn" },
		{ 0xA8E1, "_InsetRgn" },
		{ 0xA8E2, "_EmptyRgn" },
		{ 0xA8E3, "_EqualRgn" },
		{ 0xA8E4, "_SectRgn" },
		{ 0xA8E5, "_UnionRgn" },
		{ 0xA8E6, "_DiffRgn" },
		{ 0xA8E7, "_XOrRgn" },
		{ 0xA8E8, "_PtInRgn" },
		{ 0xA8E9, "_RectInRgn" },
		{ 0xA8EA, "_SetStdProcs" },
		{ 0xA8EB, "_StdBits" },
		{ 0xA8EC, "_CopyBits" },
		{ 0xA8ED, "_StdTxMeas" },
		{ 0xA8EE, "_StdGetPic" },
		{ 0xA8EF, "_ScrollRect" },
		{ 0xA8F0, "_StdPutPic" },
		{ 0xA8F1, "_StdComment" },
		{ 0xA8F2, "_PicComment" },
		{ 0xA8F3, "_OpenPicture" },
		{ 0xA8F4, "_ClosePicture" },
		{ 0xA8F5, "_KillPicture" },
		{ 0xA8F6, "_DrawPicture" },
		{ 0xA8F7, "_Layout" },
		{ 0xA8F8, "_ScalePt" },
		{ 0xA8F9, "_MapPt" },
		{ 0xA8FA, "_MapRect" },
		{ 0xA8FB, "_MapRgn" },
		{ 0xA8FC, "_MapPoly" },
		{ 0xA8FD, "_PrGlue" },
		{ 0xA8FE, "_InitFonts" },
		{ 0xA8FF, "_GetFName" },
		{ 0xA900, "_GetFNum" },
		{ 0xA901, "_FMSwapFont" },
		{ 0xA902, "_RealFont" },
		{ 0xA903, "_SetFontLock" },
		{ 0xA904, "_DrawGrowIcon" },
		{ 0xA905, "_DragGrayRgn" },
		{ 0xA906, "_NewString" },
		{ 0xA907, "_SetString" },
		{ 0xA908, "_ShowHide" },
		{ 0xA909, "_CalcVis" },
		{ 0xA90A, "_CalcVBehind" },
		{ 0xA90B, "_ClipAbove" },
		{ 0xA90C, "_PaintOne" },
		{ 0xA90D, "_PaintBehind" },
		{ 0xA90E, "_SaveOld" },
		{ 0xA90F, "_DrawNew" },
		{ 0xA910, "_GetWMgrPort" },
		{ 0xA911, "_CheckUpDate" },
		{ 0xA912, "_InitWindows" },
		{ 0xA913, "_NewWindow" },
		{ 0xA914, "_DisposeWindow" },
		{ 0xA915, "_ShowWindow" },
		{ 0xA916, "_HideWindow" },
		{ 0xA917, "_GetWRefCon" },
		{ 0xA918, "_SetWRefCon" },
		{ 0xA919, "_GetWTitle" },
		{ 0xA91A, "_SetWTitle" },
		{ 0xA91B, "_MoveWindow" },
		{ 0xA91C, "_HiliteWindow" },
		{ 0xA91D, "_SizeWindow" },
		{ 0xA91E, "_TrackGoAway" },
		{ 0xA91F, "_SelectWindow" },
		{ 0xA920, "_BringToFront" },
		{ 0xA921, "_SendBehind" },
		{ 0xA922, "_BeginUpDate" },
		{ 0xA923, "_EndUpDate" },
		{ 0xA924, "_FrontWindow" },
		{ 0xA925, "_DragWindow" },
		{ 0xA926, "_DragTheRgn" },
		{ 0xA927, "_InvalRgn" },
		{ 0xA928, "_InvalRect" },
		{ 0xA929, "_ValidRgn" },
		{ 0xA92A, "_ValidRect" },
		{ 0xA92B, "_GrowWindow" },
		{ 0xA92C, "_FindWindow" },
		{ 0xA92D, "_CloseWindow" },
		{ 0xA92E, "_SetWindowPic" },
		{ 0xA92F, "_GetWindowPic" },
		{ 0xA930, "_InitMenus" },
		{ 0xA931, "_NewMenu" },
		{ 0xA932, "_DisposeMenu" },
		{ 0xA933, "_AppendMenu" },
		{ 0xA934, "_ClearMenuBar" },
		{ 0xA935, "_InsertMenu" },
		{ 0xA936, "_DeleteMenu" },
		{ 0xA937, "_DrawMenuBar" },
		{ 0xA938, "_HiliteMenu" },
		{ 0xA939, "_EnableItem" },
		{ 0xA93A, "_DisableItem" },
		{ 0xA93B, "_GetMenuBar" },
		{ 0xA93C, "_SetMenuBar" },
		{ 0xA93D, "_MenuSelect" },
		{ 0xA93E, "_MenuKey" },
		{ 0xA93F, "_GetItmIcon" },
		{ 0xA940, "_SetItmIcon" },
		{ 0xA941, "_GetItmStyle" },
		{ 0xA942, "_SetItmStyle" },
		{ 0xA943, "_GetItmMark" },
		{ 0xA944, "_SetItmMark" },
		{ 0xA945, "_CheckItem" },
		{ 0xA946, "_GetMenuItemText" },
		{ 0xA947, "_SetMenuItemText" },
		{ 0xA948, "_CalcMenuSize" },
		{ 0xA949, "_GetMenuHandle" },
		{ 0xA94A, "_SetMFlash" },
		{ 0xA94B, "_PlotIcon" },
		{ 0xA94C, "_FlashMenuBar" },
		{ 0xA94D, "_AppendResMenu" },
		{ 0xA94E, "_PinRect" },
		{ 0xA94F, "_DeltaPoint" },
		{ 0xA950, "_CountMItems" },
		{ 0xA951, "_InsertResMenu" },
		{ 0xA952, "_DeleteMenuItem" },
		{ 0xA953, "_UpdtControl" },
		{ 0xA954, "_NewControl" },
		{ 0xA955, "_DisposeControl" },
		{ 0xA956, "_KillControls" },
		{ 0xA957, "_ShowControl" },
		{ 0xA958, "_HideControl" },
		{ 0xA959, "_MoveControl" },
		{ 0xA95A, "_GetControlReference" },
		{ 0xA95B, "_SetControlReference" },
		{ 0xA95C, "_SizeControl" },
		{ 0xA95D, "_HiliteControl" },
		{ 0xA95E, "_GetControlTitle" },
		{ 0xA95F, "_SetControlTitle" },
		{ 0xA960, "_GetControlValue" },
		{ 0xA961, "_GetControlMinimum" },
		{ 0xA962, "_GetControlMaximum" },
		{ 0xA963, "_SetControlValue" },
		{ 0xA964, "_SetControlMinimum" },
		{ 0xA965, "_SetControlMaximum" },
		{ 0xA966, "_TestControl" },
		{ 0xA967, "_DragControl" },
		{ 0xA968, "_TrackControl" },
		{ 0xA969, "_DrawControls" },
		{ 0xA96A, "_GetControlAction" },
		{ 0xA96B, "_SetControlAction" },
		{ 0xA96C, "_FindControl" },
		{ 0xA96E, "_Dequeue" },
		{ 0xA96F, "_Enqueue" },
		{ 0xA970, "_GetNextEvent" },
		{ 0xA971, "_EventAvail" },
		{ 0xA972, "_GetMouse" },
		{ 0xA973, "_StillDown" },
		{ 0xA974, "_Button" },
		{ 0xA975, "_TickCount" },
		{ 0xA976, "_GetKeys" },
		{ 0xA977, "_WaitMouseUp" },
		{ 0xA978, "_UpdtDialog" },
		{ 0xA97B, "_InitDialogs" },
		{ 0xA97C, "_GetNewDialog" },
		{ 0xA97D, "_NewDialog" },
		{ 0xA97E, "_SelectDialogItemText" },
		{ 0xA97F, "_IsDialogEvent" },
		{ 0xA980, "_DialogSelect" },
		{ 0xA981, "_DrawDialog" },
		{ 0xA982, "_CloseDialog" },
		{ 0xA983, "_DisposeDialog" },
		{ 0xA984, "_FindDialogItem" },
		{ 0xA985, "_Alert" },
		{ 0xA986, "_StopAlert" },
		{ 0xA987, "_NoteAlert" },
		{ 0xA988, "_CautionAlert" },
		{ 0xA989, "_CouldAlert" },
		{ 0xA98A, "_FreeAlert" },
		{ 0xA98B, "_ParamText" },
		{ 0xA98C, "_ErrorSound" },
		{ 0xA98D, "_GetDialogItem" },
		{ 0xA98E, "_SetDialogItem" },
		{ 0xA98F, "_SetDialogItemText" },
		{ 0xA990, "_GetDialogItemText" },
		{ 0xA991, "_ModalDialog" },
		{ 0xA992, "_DetachResource" },
		{ 0xA993, "_SetResPurge" },
		{ 0xA994, "_CurResFile" },
		{ 0xA995, "_InitResources" },
		{ 0xA996, "_RsrcZoneInit" },
		{ 0xA997, "_OpenResFile" },
		{ 0xA998, "_UseResFile" },
		{ 0xA999, "_UpdateResFile" },
		{ 0xA99A, "_CloseResFile" },
		{ 0xA99B, "_SetResLoad" },
		{ 0xA99C, "_CountResources" },
		{ 0xA99D, "_GetIndResource" },
		{ 0xA99E, "_CountTypes" },
		{ 0xA99F, "_GetIndType" },
		{ 0xA9A0, "_GetResource" },
		{ 0xA9A1, "_GetNamedResource" },
		{ 0xA9A2, "_LoadResource" },
		{ 0xA9A3, "_ReleaseResource" },
		{ 0xA9A4, "_HomeResFile" },
		{ 0xA9A5, "_SizeRsrc" },
		{ 0xA9A6, "_GetResAttrs" },
		{ 0xA9A7, "_SetResAttrs" },
		{ 0xA9A8, "_GetResInfo" },
		{ 0xA9A9, "_SetResInfo" },
		{ 0xA9AA, "_ChangedResource" },
		{ 0xA9AB, "_AddResource" },
		{ 0xA9AC, "_AddReference" },
		{ 0xA9AD, "_RmveResource" },
		{ 0xA9AE, "_RmveReference" },
		{ 0xA9AF, "_ResError" },
		{ 0xA9B0, "_WriteResource" },
		{ 0xA9B1, "_CreateResFile" },
		{ 0xA9B2, "_SystemEvent" },
		{ 0xA9B3, "_SystemClick" },
		{ 0xA9B4, "_SystemTask" },
		{ 0xA9B5, "_SystemMenu" },
		{ 0xA9B6, "_OpenDeskAcc" },
		{ 0xA9B7, "_CloseDeskAcc" },
		{ 0xA9B8, "_GetPattern" },
		{ 0xA9B9, "_GetCursor" },
		{ 0xA9BA, "_GetString" },
		{ 0xA9BB, "_GetIcon" },
		{ 0xA9BC, "_GetPicture" },
		{ 0xA9BD, "_GetNewWindow" },
		{ 0xA9BE, "_GetNewControl" },
		{ 0xA9BF, "_GetRMenu" },
		{ 0xA9C0, "_GetNewMBar" },
		{ 0xA9C1, "_UniqueID" },
		{ 0xA9C2, "_SysEdit" },
		{ 0xA9C3, "_KeyTranslate" },
		{ 0xA9C4, "_OpenRFPerm" },
		{ 0xA9C5, "_RsrcMapEntry" },
		{ 0xA9C6, "_SecondsToDate" },
		{ 0xA9C7, "_DateToSeconds" },
		{ 0xA9C8, "_SysBeep" },
		{ 0xA9C9, "_SysError" },
		{ 0xA9CA, "_PutIcon" },
		{ 0xA9CB, "_TEGetText" },
		{ 0xA9CC, "_TEInit" },
		{ 0xA9CD, "_TEDispose" },
		{ 0xA9CE, "_TETextBox" },
		{ 0xA9CF, "_TESetText" },
		{ 0xA9D0, "_TECalText" },
		{ 0xA9D1, "_TESetSelect" },
		{ 0xA9D2, "_TENew" },
		{ 0xA9D3, "_TEUpdate" },
		{ 0xA9D4, "_TEClick" },
		{ 0xA9D5, "_TECopy" },
		{ 0xA9D6, "_TECut" },
		{ 0xA9D7, "_TEDelete" },
		{ 0xA9D8, "_TEActivate" },
		{ 0xA9D9, "_TEDeactivate" },
		{ 0xA9DA, "_TEIdle" },
		{ 0xA9DB, "_TEPaste" },
		{ 0xA9DC, "_TEKey" },
		{ 0xA9DD, "_TEScroll" },
		{ 0xA9DE, "_TEInsert" },
		{ 0xA9DF, "_TESetAlignment" },
		{ 0xA9E0, "_Munger" },
		{ 0xA9E1, "_HandToHand" },
		{ 0xA9E2, "_PtrToXHand" },
		{ 0xA9E3, "_PtrToHand" },
		{ 0xA9E4, "_HandAndHand" },
		{ 0xA9E5, "_InitPack" },
		{ 0xA9E6, "_InitAllPacks" },
		{ 0xA9EB, "_FP68K" },
		{ 0xA9EC, "_Elems68K" },
		{ 0xA9EE, "_DecStr68K" },
		{ 0xA9EF, "_PtrAndHand" },
		{ 0xA9F0, "_LoadSeg" },
		{ 0xA9F1, "_UnLoadSeg" },
		{ 0xA9F2, "_Launch" },
		{ 0xA9F3, "_Chain" },
		{ 0xA9F4, "_ExitToShell" },
		{ 0xA9F5, "_GetAppParms" },
		{ 0xA9F6, "_GetResFileAttrs" },
		{ 0xA9F7, "_SetResFileAttrs" },
		{ 0xA9F8, "_MethodDispatch" },
		{ 0xA9F9, "_InfoScrap" },
		{ 0xA9FA, "_UnloadScrap" },
		{ 0xA9FB, "_LoadScrap" },
		{ 0xA9FC, "_ZeroScrap" },
		{ 0xA9FD, "_GetScrap" },
		{ 0xA9FE, "_PutScrap" },
		{ 0xA9FF, "_Debugger" },
		{ 0xAA00, "_OpenCPort" },
		{ 0xAA01, "_InitCPort" },
		{ 0xAA02, "_CloseCPort" },
		{ 0xAA03, "_NewPixMap" },
		{ 0xAA04, "_DisposePixMap" },
		{ 0xAA05, "_CopyPixMap" },
		{ 0xAA06, "_SetPortPix" },
		{ 0xAA07, "_NewPixPat" },
		{ 0xAA08, "_DisposePixPat" },
		{ 0xAA09, "_CopyPixPat" },
		{ 0xAA0A, "_PenPixPat" },
		{ 0xAA0B, "_BackPixPat" },
		{ 0xAA0C, "_GetPixPat" },
		{ 0xAA0D, "_MakeRGBPat" },
		{ 0xAA0E, "_FillCRect" },
		{ 0xAA0F, "_FillCOval" },
		{ 0xAA10, "_FillCRoundRect" },
		{ 0xAA11, "_FillCArc" },
		{ 0xAA12, "_FillCRgn" },
		{ 0xAA13, "_FillCPoly" },
		{ 0xAA14, "_RGBForeColor" },
		{ 0xAA15, "_RGBBackColor" },
		{ 0xAA16, "_SetCPixel" },
		{ 0xAA17, "_GetCPixel" },
		{ 0xAA18, "_GetCTable" },
		{ 0xAA19, "_GetForeColor" },
		{ 0xAA1A, "_GetBackColor" },
		{ 0xAA1B, "_GetCCursor" },
		{ 0xAA1C, "_SetCCursor" },
		{ 0xAA1D, "_AllocCursor" },
		{ 0xAA1E, "_GetCIcon" },
		{ 0xAA1F, "_PlotCIcon" },
		{ 0xAA20, "_OpenCPicture" },
		{ 0xAA21, "_OpColor" },
		{ 0xAA22, "_HiliteColor" },
		{ 0xAA23, "_CharExtra" },
		{ 0xAA24, "_DisposeCTable" },
		{ 0xAA25, "_DisposeCIcon" },
		{ 0xAA26, "_DisposeCCursor" },
		{ 0xAA27, "_GetMaxDevice" },
		{ 0xAA28, "_GetCTSeed" },
		{ 0xAA29, "_GetDeviceList" },
		{ 0xAA2A, "_GetMainDevice" },
		{ 0xAA2B, "_GetNextDevice" },
		{ 0xAA2C, "_TestDeviceAttribute" },
		{ 0xAA2D, "_SetDeviceAttribute" },
		{ 0xAA2E, "_InitGDevice" },
		{ 0xAA2F, "_NewGDevice" },
		{ 0xAA30, "_DisposeGDevice" },
		{ 0xAA31, "_SetGDevice" },
		{ 0xAA32, "_GetGDevice" },
		{ 0xAA33, "_Color2Index" },
		{ 0xAA34, "_Index2Color" },
		{ 0xAA35, "_InvertColor" },
		{ 0xAA36, "_RealColor" },
		{ 0xAA37, "_GetSubTable" },
		{ 0xAA38, "_UpdatePixMap" },
		{ 0xAA39, "_MakeITable" },
		{ 0xAA3A, "_AddSearch" },
		{ 0xAA3B, "_AddComp" },
		{ 0xAA3C, "_SetClientID" },
		{ 0xAA3D, "_ProtectEntry" },
		{ 0xAA3E, "_ReserveEntry" },
		{ 0xAA3F, "_SetEntries" },
		{ 0xAA40, "_QDError" },
		{ 0xAA41, "_SetWinColor" },
		{ 0xAA42, "_GetAuxWin" },
		{ 0xAA43, "_SetControlColor" },
		{ 0xAA44, "_GetAuxiliaryControlRecord" },
		{ 0xAA45, "_NewCWindow" },
		{ 0xAA46, "_GetNewCWindow" },
		{ 0xAA47, "_SetDeskCPat" },
		{ 0xAA48, "_GetCWMgrPort" },
		{ 0xAA49, "_SaveEntries" },
		{ 0xAA4A, "_RestoreEntries" },
		{ 0xAA4B, "_NewColorDialog" },
		{ 0xAA4C, "_DelSearch" },
		{ 0xAA4D, "_DelComp" },
		{ 0xAA4E, "_SetStdCProcs" },
		{ 0xAA4F, "_CalcCMask" },
		{ 0xAA50, "_SeedCFill" },
		{ 0xAA51, "_CopyDeepMask" },
		{ 0xAA52, "_HFSPinaforeDispatch" },
		{ 0xAA53, "_DictionaryDispatch" },
		{ 0xAA54, "_TextServicesDispatch" },
		{ 0xAA56, "_SpeechRecognitionDispatch" },
		{ 0xAA57, "_DockingDispatch" },
		{ 0xAA59, "_MixedModeDispatch" },
		{ 0xAA5A, "_CodeFragmentDispatch" },
		{ 0xAA5C, "_OCEUtils" },
		{ 0xAA5D, "_DigitalSignature" },
		{ 0xAA5E, "_TBDispatch" },
		{ 0xAA60, "_DeleteMCEntries" },
		{ 0xAA61, "_GetMCInfo" },
		{ 0xAA62, "_SetMCInfo" },
		{ 0xAA63, "_DisposeMCInfo" },
		{ 0xAA64, "_GetMCEntry" },
		{ 0xAA65, "_SetMCEntries" },
		{ 0xAA66, "_MenuChoice" },
		{ 0xAA67, "_ModalDialogMenuSetup" },
		{ 0xAA68, "_DialogDispatch" },
		{ 0xAA73, "_ControlDispatch" },
		{ 0xAA74, "_AppearanceDispatch" },
		{ 0xAA7E, "_SysDebugDispatch" },
		{ 0xAA80, "_AVLTreeDispatch" },
		{ 0xAA81, "_FileMappingDispatch" },
		{ 0xAA90, "_InitPalettes" },
		{ 0xAA91, "_NewPalette" },
		{ 0xAA92, "_GetNewPalette" },
		{ 0xAA93, "_DisposePalette" },
		{ 0xAA94, "_ActivatePalette" },
		{ 0xAA95, "_NSetPalette" },
		{ 0xAA96, "_GetPalette" },
		{ 0xAA97, "_PmForeColor" },
		{ 0xAA98, "_PmBackColor" },
		{ 0xAA99, "_AnimateEntry" },
		{ 0xAA9A, "_AnimatePalette" },
		{ 0xAA9B, "_GetEntryColor" },
		{ 0xAA9C, "_SetEntryColor" },
		{ 0xAA9D, "_GetEntryUsage" },
		{ 0xAA9E, "_SetEntryUsage" },
		{ 0xAA9F, "_CTab2Palette" },
		{ 0xAAA0, "_Palette2CTab" },
		{ 0xAAA1, "_CopyPalette" },
		{ 0xAAA2, "_PaletteDispatch" },
		{ 0xAAA3, "_CodecDispatch" },
		{ 0xAAA4, "_ALMDispatch" },
		{ 0xAADB, "_CursorDeviceDispatch" },
		{ 0xAAF2, "_ControlStripDispatch" },
		{ 0xAAF3, "_ExpansionManager" },
		{ 0xAB1D, "_QDExtensions" },
		{ 0xABC3, "_NQDMisc" },
		{ 0xABC9, "_IconDispatch" },
		{ 0xABCA, "_DeviceLoop" },
		{ 0xABEB, "_DisplayDispatch" },
		{ 0xABED, "_DragDispatch" },
		{ 0xABF1, "_GestaltValueDispatch" },
		{ 0xABF2, "_ThreadDispatch" },
		{ 0xABF6, "_CollectionMgr" },
		{ 0xABF8, "_StdOpcodeProc" },
		{ 0xABFC, "_TranslationDispatch" },
		{ 0xABFF, "_DebugStr" }
	};

	int i;

	for (i = 0; i < std::size(traps); i++)
	{
		if (traps[i].trap == opcode)
			return traps[i].name;
	}
	return nullptr;
}



offs_t mac68k_dasm_override(std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)
{
	uint16_t opcode;
	unsigned result = 0;
	const char *trap;

	opcode = opcodes.r16(pc);
	if ((opcode & 0xF000) == 0xA000)
	{
		trap = lookup_trap(opcode);
		if (trap != nullptr)
		{
			stream << trap;
			result = 2 | util::disasm_interface::STEP_OVER | util::disasm_interface::SUPPORTED;
		}
	}
	return result;
}
