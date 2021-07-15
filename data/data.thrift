

struct Student{
    1: i32 sno,
    2: string sname,
    3: bool ssex,
    4: i16 sage,
}

service Serv{
     void put(1: Student s),
     i32 icall(1: Student s),
     string scall(1: Student s),
     /*
         string& srcall(1: Student s),
         -----------------------------
         -thrift -r --gen cpp student.thrift
         -error:
         -   [ERROR:/root/test/thrift/student.thrift:12] (last token was '&')
         -   syntax error
         -   [FAILURE:/root/test/thrift/student.thrift:12] Parser error during include pass.
         -----------------------------
     */
     Student stcall(1: Student s),
}