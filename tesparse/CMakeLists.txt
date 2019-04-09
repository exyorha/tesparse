add_library(tesparse STATIC
	include/tesparse/Expression.h
	include/tesparse/ExpressionEvaluator.h
	include/tesparse/ExpressionParser.h
	include/tesparse/FileMapping.h
	include/tesparse/FourCC.h
	include/tesparse/InputSerializationStream.h
	include/tesparse/OutputSerializationStream.h
	include/tesparse/SerializationStream.h
	include/tesparse/StringConversions.h
	include/tesparse/TESFileFormatDescription.h
	include/tesparse/TESGameData.h
	include/tesparse/TESValue.h
	include/tesparse/WindowsHandle.h
	tesparse/ExpressionEvaluator.cpp
	tesparse/ExpressionParser.cpp
	tesparse/FileMapping.cpp
	tesparse/FourCC.cpp
	tesparse/InputSerializationStream.cpp
	tesparse/OutputSerializationStream.cpp
	tesparse/SerializationStream.cpp
	tesparse/StringConversions.cpp
	tesparse/TESFileFormatDescription.cpp
	tesparse/TESGameData.cpp
	tesparse/WindowsHandle.cpp
)

target_include_directories(tesparse PUBLIC include)
target_compile_definitions(tesparse PUBLIC -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_VC_EXTRALEAN -DUNICODE -D_UNICODE)
target_link_libraries(tesparse PUBLIC shlwapi xmllite)
