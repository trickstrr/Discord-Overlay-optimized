#pragma once
#include <Windows.h>
#include <string>
#include <immintrin.h>

namespace OverlayCord {
    namespace Communication {
        struct Header {
            UINT Magic;
            UINT FrameCount;
            UINT Reserved;
            UINT Width;
            UINT Height;
            BYTE Buffer[1];
        };

        struct ConnectedProcessInfo {
            UINT ProcessId;
            HANDLE File;
            Header* MappedAddress;
        };

        inline bool ConnectToProcess(ConnectedProcessInfo& processInfo) {
            std::string mappedFilename = "DiscordOverlay_Framebuffer_Memory_" + std::to_string(processInfo.ProcessId);
            processInfo.File = OpenFileMappingA(FILE_MAP_ALL_ACCESS, false, mappedFilename.c_str());
            if (!processInfo.File || processInfo.File == INVALID_HANDLE_VALUE)
                return false;

            processInfo.MappedAddress = static_cast<Header*>(MapViewOfFile(processInfo.File, FILE_MAP_ALL_ACCESS, 0, 0, 0));
            return processInfo.MappedAddress != nullptr;
        }

        inline void DisconnectFromProcess(ConnectedProcessInfo& processInfo) {
            if (processInfo.MappedAddress) {
                UnmapViewOfFile(processInfo.MappedAddress);
                processInfo.MappedAddress = nullptr;
            }
            if (processInfo.File) {
                CloseHandle(processInfo.File);
                processInfo.File = nullptr;
            }
        }





        inline void SendFrame(ConnectedProcessInfo& processInfo, UINT width, UINT height, void* frame, UINT size) {
            if (processInfo.MappedAddress) {
                processInfo.MappedAddress->Width = width;
                processInfo.MappedAddress->Height = height;
                void* dest = processInfo.MappedAddress->Buffer;
                const void* src = frame;


                if (IsProcessorFeaturePresent(PF_AVX2_INSTRUCTIONS_AVAILABLE)) {
                    size_t i = 0;
                    for (; i + 32 <= size; i += 32) {
                        __m256i data = _mm256_loadu_si256((__m256i*)((char*)src + i));
                        _mm256_storeu_si256((__m256i*)((char*)dest + i), data);
                    }
                    for (; i < size; ++i) {
                        ((char*)dest)[i] = ((char*)src)[i];
                    }
                }
                else if (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {
                    size_t i = 0;
                    for (; i + 16 <= size; i += 16) {
                        __m128i data = _mm_loadu_si128((__m128i*)((char*)src + i));
                        _mm_storeu_si128((__m128i*)((char*)dest + i), data);
                    }
                    for (; i < size; ++i) {
                        ((char*)dest)[i] = ((char*)src)[i];
                    }
                }
                else {
                    memcpy(dest, src, size);
                }

                processInfo.MappedAddress->FrameCount++;
            }
        }

        
    }
}

