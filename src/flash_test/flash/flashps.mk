
flashps.dll: dlldata.obj flash_p.obj flash_i.obj
	link /dll /out:flashps.dll /def:flashps.def /entry:DllMain dlldata.obj flash_p.obj flash_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \
.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del flashps.dll
	@del flashps.lib
	@del flashps.exp
	@del dlldata.obj
	@del flash_p.obj
	@del flash_i.obj
