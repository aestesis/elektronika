
dxsendps.dll: dlldata.obj dxsend_p.obj dxsend_i.obj
	link /dll /out:dxsendps.dll /def:dxsendps.def /entry:DllMain dlldata.obj dxsend_p.obj dxsend_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del dxsendps.dll
	@del dxsendps.lib
	@del dxsendps.exp
	@del dlldata.obj
	@del dxsend_p.obj
	@del dxsend_i.obj
