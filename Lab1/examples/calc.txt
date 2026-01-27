method main()
var a: int;
    b: int;
    next: int;
    i: int;
    n: int;
begin
    a := 0;
    b := 1;
    i := 0;
    n := 10;

loop:
    if i < n then begin
        print_int(a);

        next := a + b;
        a := b;
        b := next;

        i := i + 1;
        goto loop;
    end;
end;
