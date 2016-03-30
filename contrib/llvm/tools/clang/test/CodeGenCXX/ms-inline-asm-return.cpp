// REQUIRES: x86-registered-target
// RUN: %clang_cc1 %s -triple i686-pc-windows-msvc -emit-llvm -o - -fasm-blocks | FileCheck %s

// Check that we take EAX or EAX:EDX and return it from these functions for MSVC
// compatibility.

extern "C" {

long long f_i64() {
  __asm {
    mov eax, 1
    mov edx, 1
  }
}
// CHECK-LABEL: define i64 @f_i64()
// CHECK: %[[r:[^ ]*]] = call i64 asm sideeffect inteldialect "mov eax, $$1\0A\09mov edx, $$1", "=A,~{eax},{{.*}}"
// CHECK: ret i64 %[[r]]

int f_i32() {
  __asm {
    mov eax, 1
    mov edx, 1
  }
}
// CHECK-LABEL: define i32 @f_i32()
// CHECK: %[[r:[^ ]*]] = call i32 asm sideeffect inteldialect "mov eax, $$1\0A\09mov edx, $$1", "={eax},~{eax},{{.*}}"
// CHECK: ret i32 %[[r]]

short f_i16() {
  __asm {
    mov eax, 1
    mov edx, 1
  }
}
// CHECK-LABEL: define signext i16 @f_i16()
// CHECK: %[[r:[^ ]*]] = call i32 asm sideeffect inteldialect "mov eax, $$1\0A\09mov edx, $$1", "={eax},~{eax},{{.*}}"
// CHECK: %[[r_i16:[^ ]*]] = trunc i32 %[[r]] to i16
// CHECK: ret i16 %[[r_i16]]

char f_i8() {
  __asm {
    mov eax, 1
    mov edx, 1
  }
}
// CHECK-LABEL: define signext i8 @f_i8()
// CHECK: %[[r:[^ ]*]] = call i32 asm sideeffect inteldialect "mov eax, $$1\0A\09mov edx, $$1", "={eax},~{eax},{{.*}}"
// CHECK: %[[r_i8:[^ ]*]] = trunc i32 %[[r]] to i8
// CHECK: ret i8 %[[r_i8]]

bool f_i1() {
  __asm {
    mov eax, 1
    mov edx, 1
  }
}
// CHECK-LABEL: define zeroext i1 @f_i1()
// CHECK: %[[r:[^ ]*]] = call i32 asm sideeffect inteldialect "mov eax, $$1\0A\09mov edx, $$1", "={eax},~{eax},{{.*}}"
// CHECK: %[[r_i8:[^ ]*]] = trunc i32 %[[r]] to i8
// CHECK: store i8 %[[r_i8]], i8* %{{.*}}
// CHECK: %[[r_i1:[^ ]*]] = load i1, i1* %{{.*}}
// CHECK: ret i1 %[[r_i1]]

struct FourChars {
  char a, b, c, d;
};
FourChars f_s4() {
  __asm {
    mov eax, 0x01010101
  }
}
// CHECK-LABEL: define i32 @f_s4()
// CHECK: %[[r:[^ ]*]] = call i32 asm sideeffect inteldialect "mov eax, $$0x01010101", "={eax},~{eax},{{.*}}"
// CHECK: store i32 %[[r]], i32* %{{.*}}
// CHECK: %[[r_i32:[^ ]*]] = load i32, i32* %{{.*}}
// CHECK: ret i32 %[[r_i32]]

struct EightChars {
  char a, b, c, d, e, f, g, h;
};
EightChars f_s8() {
  __asm {
    mov eax, 0x01010101
    mov edx, 0x01010101
  }
}
// CHECK-LABEL: define i64 @f_s8()
// CHECK: %[[r:[^ ]*]] = call i64 asm sideeffect inteldialect "mov eax, $$0x01010101\0A\09mov edx, $$0x01010101", "=A,~{eax},{{.*}}"
// CHECK: store i64 %[[r]], i64* %{{.*}}
// CHECK: %[[r_i64:[^ ]*]] = load i64, i64* %{{.*}}
// CHECK: ret i64 %[[r_i64]]

} // extern "C"

int main() {
  __asm xor eax, eax
}
// CHECK-LABEL: define i32 @main()
// CHECK: %[[r:[^ ]*]] = call i32 asm sideeffect inteldialect "xor eax, eax", "={eax},{{.*}}"
// CHECK: ret i32 %[[r]]
