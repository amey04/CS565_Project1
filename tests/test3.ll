; ModuleID = 'test3.bc'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.9.0"

; Function Attrs: nounwind ssp uwtable
define i32 @main() #0 {
entry:
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 12, i32* %x, align 4
  %0 = load i32* %x, align 4
  %add = add nsw i32 %0, 22
  store i32 %add, i32* %y, align 4
  %1 = load i32* %y, align 4
  %add1 = add nsw i32 %1, 33
  store i32 %add1, i32* %z, align 4
  ret i32 0
}

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.0 (trunk 203094) (llvm/trunk 203092)"}
