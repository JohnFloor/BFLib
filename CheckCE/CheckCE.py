import re
import os
import sys
import subprocess
import time
import threading
from pathlib import Path
from subprocess import CompletedProcess
from typing import Callable
from typing import Optional
from concurrent.futures import ThreadPoolExecutor


argPath                 = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
argSolutionConf         =      sys.argv[2] if len(sys.argv) > 2 else "Debug"
argNWorkerThreads       =      sys.argv[3] if len(sys.argv) > 3 else "10"

cScriptName             = Path(sys.argv[0]).stem

cStartTime              = time.perf_counter_ns()

gError                  = None
cErrorFailed            = 1
cErrorTechnical         = 2

cCppExt                 = ".cpp"
cCompilationErrorTagLog = "[CompilationError-*]"

reInvalidConf           = re.compile(r'error MSB4126: The specified solution configuration ".*\|.*" is invalid.')
reClExe                 = re.compile(r"\bCL\.exe\b", re.IGNORECASE)
reClExeFullPath         = re.compile(r"(^.*\bCL\.exe\b)", re.IGNORECASE)
rePDBSwitches           = re.compile(r"/Z[7iI] ")
reCompilationErrorTag   = re.compile(r"\[CompilationError(?:-(\w*))?\]")
reCompilationErrorLine  = re.compile(r"(\s*)//([^/])(.*[^/]//\s*" + reCompilationErrorTag.pattern + r":" + r"\s*(.*\S)\s*)")
reDiagnosticLine        = re.compile(r"^..[^:]*\([0-9,]+\): ([a-z ]+) [A-Z]+\d+: (.*)")

gLoggedTitles           = set()				# the titles that have been already logged
gPrintLock              = threading.Lock()	# one common lock for printing, using 'gError' and 'gLoggedTitles'


def PrintUsage() -> None:
	print(f"Checks '{cCompilationErrorTagLog}' tags in .cpp files.")
	print(r"")
	print(r"Usage:")
	print(f"    {cScriptName}.py")
	print(f"    {cScriptName}.py path")
	print(f"    {cScriptName}.py path solution_conf")
	print(f"    {cScriptName}.py path solution_conf n_worker_threads")
	print(r"        path:             A directory or a single .cpp file to process.")
	print(r"                          Default value: '.' (the current directory).")
	print(r"        solution_conf:    A solution configuration. Must be present in the .sln file.")
	print(r"                          Default value: 'Debug'.")
	print(r"        n_worker_threads: Number of worker threads to use for the check, between 1-20.")
	print(r"                          Default value: '10'.")
	print(r"")
	print(r"    The .sln file will be searched in the following directories: path, path\.., path\..\.., etc.")
	print(r"    The search stops at the first match. It is an error if that directory contains more than one .sln file.")
	print(r"")
	print(r"    A C++ project directory is a directory in which there is a .vcxproj file. It is an error, if it contains")
	print(r"    more than one .vcxproj file. It must be the same directory or a child of the solution directory.")
	print(r"")
	print(r"    'path' must be in a project directory (descendant-or-self).")
	print(r"")
	print(r"Print this help:")
	print(f"    {cScriptName}.py /?")
	print(r"")
	print(r"Exit codes:")
	print(r"    0: The check passed for all (0 or more) .cpp files.")
	print(r"    1: The check failed at least once.")
	print(r"    2: Technical error (e.g. directory/file does not exist).")


def QR(dividend: int, divisor: int) -> tuple[int, int]:
	return dividend // divisor, dividend % divisor


def AssertIsSingleThreadedCode() -> None:
	assert threading.current_thread() is threading.main_thread(), "This function should be called from the main thread."
	assert len(threading.enumerate()) == 1, "Only the main thread should run, when this function is called."


def Exit():
	AssertIsSingleThreadedCode()		# because it may terminate the program

	elapsedTime = time.perf_counter_ns() - cStartTime
	hours,   r1 = QR(elapsedTime, 1_000_000_000 * 60 * 60)
	minutes, r2 = QR(r1,          1_000_000_000 * 60)
	seconds     =    r2     /     1_000_000_000
	print("")
	print(f"Time Elapsed {hours:02d}:{minutes:02d}:{seconds:05.2f}")

	if gError is None:
		sys.exit(0)
	else:
		sys.exit(gError)


def GetDirFromFile(path: Path) -> Path:
	if path.is_file():
		return path.parent
	else:
		return path


def PrintToStdErr(s: str) -> None:
	sys.stdout.flush()
	sys.stderr.flush()
	print(s, file = sys.stderr)
	sys.stdout.flush()
	sys.stderr.flush()


def ExitErrorTechnical(message: str) -> None:
	AssertIsSingleThreadedCode()		# because it may terminate the program

	global gError
	PrintToStdErr(f"ERROR: {message}")
	assert gError is None or gError == cErrorTechnical, f"gError should be None or cErrorTechnical."
	gError = cErrorTechnical

	Exit()


def PrintErrorFailed(file: Path, lineInd: int, tag: str, message: str, *args) -> None:
	assert len(args) % 4 == 0, "The number of arguments has to be a multiple of 4."

	def PrintOneError(file: Path, lineInd: int, tag: str, message: str) -> None:
		assert tag in ("error", "warning", "message"), f"Unrecognized tag value '{tag}'."
		PrintToStdErr(f"{file}({lineInd + 1}): {tag:7}: {message}")

	with gPrintLock:
		PrintOneError(file, lineInd, tag, message)
		for i in range(0, len(args), 4):
			file, lineInd, tag, message = args[i : i + 4]
			PrintOneError(file, lineInd, tag, message)

		global gError
		assert gError is None or gError == cErrorFailed, f"gError should be None or cErrorFailed."
		gError = cErrorFailed


def PrintTitle(path: Path) -> None:
	title = str(path.relative_to(GetDirFromFile(argPath)))
	with gPrintLock:
		if title not in gLoggedTitles:
			print(f"    Processing {title}...")
			gLoggedTitles.add(title)


def IsCppExt(path: Path) -> bool:
	return path.suffix.lower() == cCppExt


def GetCppReAtEnd(clExeCommand: str) -> re.Pattern:
	if clExeCommand.endswith("\""):
		return re.compile(r'"[^"]+"$', re.IGNORECASE)
	else:
		return re.compile(r"\S+$", re.IGNORECASE)


def GetSmallestCppFile(directory: Path) -> Optional[Path]:
	smallestPath = None
	smallestSize = 2**62 - 1			# largest single-limb int

	for path in directory.rglob("*" + cCppExt):
		if path.is_file():
			size = path.stat().st_size
			if size < smallestSize:
				smallestSize = size
				smallestPath = path

	return smallestPath


def ConvertArgNWorkerThreads() -> None:
	AssertIsSingleThreadedCode()		# because it may terminate the program

	try:
		global argNWorkerThreads
		argNWorkerThreads = int(argNWorkerThreads)
		if argNWorkerThreads < 1 or argNWorkerThreads > 20:
			ExitErrorTechnical(f"The given number of worker threads '{argNWorkerThreads}' is out of range. Should be between 1-20.")
	except ValueError:
		ExitErrorTechnical(f"The given number of worker threads '{argNWorkerThreads}' is not an integer.")


def FindSolutionDir(startDir: Path) -> Path:
	AssertIsSingleThreadedCode()		# because it may terminate the program
	assert startDir.is_dir(), f"'{startDir}' should be a directory."

	for d in [startDir] + list(startDir.parents):
		slnCount = sum(1 for sln in d.glob("*.sln") if sln.is_file())
		if slnCount == 1:
			return d
		if slnCount >= 2:
			ExitErrorTechnical(f"The directory '{d}' contains more than one .sln file.")

	ExitErrorTechnical(f"No directory with a .sln file found while walking up from '{startDir}'.")


def FindProjectDir(startDir: Path, solutionDir: Path) -> Path:
	AssertIsSingleThreadedCode()		# because it may terminate the program
	assert startDir.is_dir(), f"'{startDir}' should be a directory."
	assert solutionDir.is_dir(), f"'{solutionDir}' should be a directory."
	assert startDir.is_relative_to(solutionDir), f"'{startDir}' should be in '{solutionDir}' (descendant-or-self)."

	d = startDir
	while True:
		vcxprojCount = sum(1 for proj in d.glob("*.vcxproj") if proj.is_file())
		if vcxprojCount == 1:
			if d == solutionDir or d.parent == solutionDir:
				return d
			else:
				ExitErrorTechnical(f"The C++ project directory '{d}' is not the same or the child of the solution directory '{solutionDir}'.")
		if vcxprojCount >= 2:
			ExitErrorTechnical(f"The directory '{d}' contains more than one .vcxproj file.")
		if d == solutionDir:
			break
		d = d.parent

	ExitErrorTechnical(f"No .vcxproj file found while walking up from '{startDir}' to '{solutionDir}'.")


def TouchExistingFile(file: Path) -> None:
	os.utime(file)


def TouchSmallestCppFile(directory: Path) -> None:
	AssertIsSingleThreadedCode()		# because it may terminate the program

	smallestPath = GetSmallestCppFile(directory)
	if smallestPath is None:
		ExitErrorTechnical(f"No .cpp file in directory '{directory}'.")
	TouchExistingFile(smallestPath)


def RunSubprocess(commandLine:      str | list[str],
				  workingDirectory: Path = None,
				  inShell:          str = "NoShell",
				  environment:      dict[str, str] = None) -> CompletedProcess:
	assert inShell in ("NoShell", "InShell"), f"Unrecognized inShell value '{inShell}'."

	return subprocess.run(commandLine,
						  cwd            = workingDirectory,
						  capture_output = True,                 # capture stdout and stderr
						  text           = True,                 # output as 'str', not 'bytes'
						  encoding       = "utf-8",              # how the stdout is encoded
						  shell          = inShell == "InShell", # interpret the command with cmd.exe?
						  env            = environment)          # environment variables


def ChangeTerminalToUtf8() -> None:
	chcpResult = RunSubprocess("CHCP 65001", None, "InShell")
	assert chcpResult.returncode == 0, "Failed to change code page in terminal."


def GetDeveloperEnv() -> dict[str, str]:
	vcVarsPath   = Path(r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat")
	vcVarsResult = RunSubprocess(f"{vcVarsPath.name} >nul && set", vcVarsPath.parent, "InShell")
	
	env = os.environ.copy()
	for line in vcVarsResult.stdout.splitlines():
		if '=' in line:
			key, value = line.split('=', 1)
			env[key] = value
	
	return env


def BuildSolution(solutionDir: Path, mode: str) -> CompletedProcess:
	commandLine = [r"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe",
				   f"/p:Configuration={argSolutionConf}",
					"/p:Platform=x64"]

	match mode:
		case "Full":    pass
		case "JustObj": commandLine.append("/t:ClCompile")
		case _:         assert False, f"Unrecognized mode value '{mode}'."

	return RunSubprocess(commandLine, solutionDir)
	# {return value}.stdout: all output goes here, its type is 'str'
	# {return value}.returncode = 0 means build succeeded
	# {return value}.returncode = 1 means build failed


def GetCompileFunction(dirToTouch: Path, developerEnv: dict[str, str]) -> Callable[[Path], CompletedProcess]:
	AssertIsSingleThreadedCode()		# because it may terminate the program

	solutionDir = FindSolutionDir(dirToTouch)
	projectDir  = FindProjectDir(dirToTouch, solutionDir)

	bsFull = BuildSolution(solutionDir, "Full")
	if bsFull.returncode != 0:
		if re.search(reInvalidConf, bsFull.stdout):
			ExitErrorTechnical(f"The solution does not contain '{argSolutionConf}' as a solution configuration.")
		else:
			ExitErrorTechnical("The solution did not build successfully.")

	TouchSmallestCppFile(dirToTouch)
	bsJustObj = BuildSolution(solutionDir, "JustObj")
	assert bsJustObj.returncode == 0, "The solution should build successfully."
	stdoutLines = bsJustObj.stdout.splitlines()

	clExes = [s for s in stdoutLines if reClExe.search(s)]
	assert len(clExes) == 1, "The number of CL.exe calls in the output of MSBuild should be 1."
	clExe = clExes[0]
	clExe = clExe.lstrip()
	assert clExe.lower().count("cl.exe") == 1, "The number of 'CL.exe'-s in the CL.exe call should be 1."
	assert clExe.lower().count(cCppExt) == 1, "The number of '.cpp'-s in the CL.exe call should be 1."
	assert not clExe.startswith("\""), "The CL.exe call should not start with \"."
	clExe = reClExeFullPath.sub(r'"\1"', clExe)		# put the full path to CL.exe into ""
	clExe = GetCppReAtEnd(clExe).sub("", clExe)		# remove Something.cpp or "Some thing.cpp" at the end, space remains
	clExe = rePDBSwitches.sub("", clExe)			# don't generate PDB either into the .obj file or into a .pdb file
	clExe += "/Zs"									# syntax check only, no output files will be created

	def CompileOneFile(path: Path) -> CompletedProcess:
		assert path.is_absolute(), f"'{path}' should be absolute."
		assert path.is_relative_to(projectDir), f"'{path}' should be in '{projectDir}' (descendant-or-self)."
		return RunSubprocess(f"{clExe} \"{path.relative_to(projectDir)}\"", projectDir, "NoShell", developerEnv)

	return CompileOneFile


def ProcessCompilationErrorTag(path: Path, compileOneFile: Callable[[Path], CompletedProcess], lines: list[str], lineInd: int) -> None:
	taggedLine = lines[lineInd]
	tagOccurrences = len(re.findall(reCompilationErrorTag, taggedLine))
	assert tagOccurrences >= 1, f"'{cCompilationErrorTagLog}' should occur on the tagged line."

	PrintTitle(path)

	if tagOccurrences > 1:
		PrintErrorFailed(path, lineInd, "error", f"'{cCompilationErrorTagLog}' occurs more than once on the line.")
		return

	mo = re.fullmatch(reCompilationErrorLine, taggedLine)
	if mo is None:
		PrintErrorFailed(path, lineInd, "error", f"'{cCompilationErrorTagLog}' is in the line, but the line's format is incorrect.")
		return

	uncommentedLine    = mo.group(1) + ("" if mo.group(2) == " " else mo.group(2)) + mo.group(3)
	onSolutionConf     = mo.group(4)
	expectedDiagnostic = mo.group(5)

	if onSolutionConf is not None:
		if onSolutionConf == "":
			PrintErrorFailed(path, lineInd, "error", f"The solution configuration in '{cCompilationErrorTagLog}' is empty.")
			return
		if onSolutionConf != argSolutionConf:
			return

	try:
		outPath = path.parent / f"{path.stem}.{cScriptName}-{lineInd + 1}{path.suffix}"

		with outPath.open("w", encoding = "utf-8") as f:
			f.writelines(lines[0 : lineInd])
			f.write(uncommentedLine)
			f.writelines(lines[lineInd + 1 : ])

		cl = compileOneFile(outPath)
		if cl.returncode == 0:
			PrintErrorFailed(path, lineInd, "error", "Compilation succeeded.")			# 'cl.stdout' can have warnings
			return

		clDiagnosticLine = cl.stdout.splitlines()[1]									# 0th line is the name of the .cpp

		mo = re.fullmatch(reDiagnosticLine, clDiagnosticLine)
		assert mo is not None, "Unrecognized diagnostic line format."

		clDiagnosticType = mo.group(1)
		clDiagnostic     = mo.group(2)

		if clDiagnosticType != "error":
			PrintErrorFailed(path, lineInd, "error",   f"The first diagnostic is a {clDiagnosticType}.",
							 path, lineInd, "message", f"Expected error:    {expectedDiagnostic}",
							 path, lineInd, "message", f"Actual diagnostic: {clDiagnostic}")
			return

		if expectedDiagnostic not in clDiagnostic:
			PrintErrorFailed(path, lineInd, "error",   f"Expected error not found.",
							 path, lineInd, "message", f"Expected error: {expectedDiagnostic}",
							 path, lineInd, "message", f"Actual error:   {clDiagnostic}")
			return
	finally:
		outPath.unlink()


def ProcessCppFile(pool: ThreadPoolExecutor, path: Path, compileOneFile: Callable[[Path], CompletedProcess]) -> None:
	assert path.is_file(), f"'{path}' should be an existing file."
	assert IsCppExt(path), f"'{path}' should have '.cpp' extension."

	lines = path.read_text(encoding = "utf-8").splitlines(keepends = True)

	foundTag = False

	for i in range(len(lines)):
		if re.search(reCompilationErrorTag, lines[i]):
			pool.submit(ProcessCompilationErrorTag, path, compileOneFile, lines, i)
			foundTag = True

	if not foundTag:
		pool.submit(PrintTitle, path)


# === Main =============================================================================================================

if len(sys.argv) == 2 and sys.argv[1] == "/?":
	PrintUsage()
	sys.exit()

if len(sys.argv) > 4:
	ExitErrorTechnical("Too many arguments.")

ConvertArgNWorkerThreads()

if argPath.is_dir():
	pass
elif argPath.is_file():
	if not IsCppExt(argPath):
		ExitErrorTechnical(f"The specified file '{argPath}' is not a .cpp file.")
else:
	ExitErrorTechnical(f"The path '{argPath}' is not an existing directory or file.")

ChangeTerminalToUtf8()
developerEnv   = GetDeveloperEnv()
compileOneFile = GetCompileFunction(GetDirFromFile(argPath), developerEnv)

with ThreadPoolExecutor(max_workers = argNWorkerThreads) as pool:
	if argPath.is_dir():
		for path in argPath.rglob("*" + cCppExt):
			if path.is_file():
				ProcessCppFile(pool, path, compileOneFile)
	elif argPath.is_file():
		ProcessCppFile(pool, argPath, compileOneFile)

Exit()
