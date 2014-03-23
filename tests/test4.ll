; ModuleID = 'test4.bc'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.9.0"

@.str = private unnamed_addr constant [26 x i8] c"This code is unreachable\0A\00", align 1
@.str1 = private unnamed_addr constant [57 x i8] c"This code is reachable from label2, but not from entry.\0A\00", align 1
@.str2 = private unnamed_addr constant [30 x i8] c"Expected result: 5*3*2*5=150\0A\00", align 1
@.str3 = private unnamed_addr constant [3 x i8] c"%d\00", align 1

; Function Attrs: nounwind ssp uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %n = alloca i32, align 4
  store i32 0, i32* %retval
  store i32 5, i32* %n, align 4
  br label %label5

label0:                                           ; No predecessors!
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([26 x i8]* @.str, i32 0, i32 0))
  store i32 0, i32* %n, align 4
  br label %label1

label1:                                           ; preds = %label5, %label0
  %0 = load i32* %n, align 4
  %mul = mul nsw i32 %0, 2
  store i32 %mul, i32* %n, align 4
  br label %label4

label2:                                           ; No predecessors!
  %call1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([26 x i8]* @.str, i32 0, i32 0))
  store i32 0, i32* %n, align 4
  br label %label13

label13:                                          ; preds = %label2
  store i32 2, i32* %n, align 4
  br label %label3

label3:                                           ; preds = %label13
  %call2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([57 x i8]* @.str1, i32 0, i32 0))
  store i32 0, i32* %n, align 4
  br label %label4

label4:                                           ; preds = %label3, %label1
  %1 = load i32* %n, align 4
  %mul3 = mul nsw i32 %1, 5
  store i32 %mul3, i32* %n, align 4
  br label %end

label5:                                           ; preds = %entry
  %2 = load i32* %n, align 4
  %mul4 = mul nsw i32 %2, 3
  store i32 %mul4, i32* %n, align 4
  br label %label1

end:                                              ; preds = %label4
  %call5 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([30 x i8]* @.str2, i32 0, i32 0))
  %3 = load i32* %n, align 4
  %call6 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.str3, i32 0, i32 0), i32 %3)
  %4 = load i32* %retval
  ret i32 %4
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { nounwind ssp uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.0 (trunk 203094) (llvm/trunk 203092)"}
