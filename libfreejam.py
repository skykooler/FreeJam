# TODO: un-hardcode this path
LIB_PATH = "./target/debug/libfreejam.so"

import cffi
import os
import time

class AudioSystem:
    def __init__(self):
        ffi = cffi.FFI()

        self._lib = ffi.dlopen(LIB_PATH)
        ffi.cdef("""
        void* audioloop();
        void* play(void *stream);
        void* pause(void *stream);
        void* play_note(void *stream, uint8_t note, uint8_t velocity);
        void* stop_note(void *stream, uint8_t note);
        """)

        self._stream = self._lib.audioloop()
    
    def pause(self):
        self._stream = self._lib.pause(self._stream)
    
    def play(self):
        self._stream = self._lib.play(self._stream)
    
    def play_note(self, note, velocity):
        self._stream = self._lib.play_note(self._stream, note, velocity)
    
    def stop_note(self, note):
        self._stream = self._lib.stop_note(self._stream, note)
    
    def stop_all_notes(self):
        for i in range(128):
            self._stream = self._lib.stop_note(self._stream, i)

    def hit_note(self, note, velocity, ms):
        self.play_note(note, velocity)
        time.sleep(ms/1000)
        self.stop_note(note)

if __name__=="__main__":
    asys = AudioSystem()