#ifndef COMMONDATA_H
#define COMMONDATA_H

// main parameters
// simple parameters - int, long, double, etc.
#define DeclareParRO(Type, Name) \
    protected: \
        Type m##Name; \
    public: \
        Type Name() const { return m##Name; }

#define DeclarePar(Type, Name) \
        DeclareParRO(Type, Name) \
    protected: \
        Type m##Name##Orig; \
    public: \
        void set##Name(Type a##Name, bool aSetOrig = false) { \
            m##Name = a##Name; \
            if (aSetOrig) m##Name##Orig = a##Name; \
        }

// complex parameters - QString, QDateTime, QList, etc.
#define DefParComplRO(Type, Name) \
    protected: \
        Type m##Name; \
    public: \
        const Type & Name##Const() const { return m##Name; }

#define DefParComplRORef(Type, Name) \
        DefParComplRO(Type, Name) \
        Type & Name##Ref() { return m##Name; }

#define DefParCompl(Type, Name) \
    DefParComplRORef(Type, Name) \
    protected: \
        Type m##Name##Orig; \
    public: \
        void set##Name(Type a##Name, bool aSetOrig = false) { \
            m##Name = a##Name; \
            if (aSetOrig) m##Name##Orig = a##Name; \
        }

// QString - as complex parameter
#define DefParStrRO(Name) DefParComplRO(QString, Name)
#define DefParStr(Name) DefParCompl(QString, Name)



//------------------------------------------------------------------------------------
// parameters with Group Num
// simple parameters - int, long, double, etc.
#define DefParRONum(Type, Name) \
    protected: \
        Type m##Name; \
    public: \
        Type Name() { \
            if (!mInited.contains(DEFINE_GROUP_NUM)) InitGrpData(DEFINE_GROUP_NUM); \
            return m##Name; \
        } \

#define DefParNum(Type, Name) \
    DefParRONum(Type, Name) \
    protected: \
        Type m##Name##Orig; \
    public: \
        void set##Name(Type a##Name, bool aSetOrig = false) { \
            m##Name = a##Name; \
            if (aSetOrig) m##Name##Orig = a##Name; \
        }

// complex parameters - QString, QDateTime, QList, etc.
#define DefParComplRONum(Type, Name) \
    protected: \
        Type m##Name; \
    public: \
        const Type & Name##Const() { \
            if (!mInited.contains(DEFINE_GROUP_NUM)) InitGrpData(DEFINE_GROUP_NUM); \
            return m##Name; \
        }

#define DefParComplNum(Type, Name) \
    DefParComplRONum(Type, Name) \
    public: \
        Type & Name##Ref() { \
            if (!mInited.contains(DEFINE_GROUP_NUM)) InitGrpData(DEFINE_GROUP_NUM); \
            return m##Name; \
        } \
        void set##Name(Type a##Name) { \
            m##Name = a##Name; \
        }

//#define DefParComplNum(Type, Name) \
//    DefParComplRONum(Type, Name) \
//    protected: \
//        Type m##Name##Orig; \
//    public: \
//        Type & Name##Ref() { \
//            if (!mInited.contains(DEFINE_GROUP_NUM)) InitGrpData(DEFINE_GROUP_NUM); \
//            return m##Name; \
//        } \
//        void set##Name(Type a##Name, bool aSetOrig = false) { \
//            m##Name = a##Name; \
//            if (aSetOrig) m##Name##Orig = a##Name; \
//        }

// String with group num
#define DefParStrRONum(Name) DefParComplRONum(QString, Name)
#define DefParStrNum(Name) DefParComplNum(QString, Name)

// for use in constructors
#define InitPar(Name) m##Name(a##Name), m##Name##Orig(a##Name)
#define InitParRO(Name) m##Name(a##Name)

// for use in SetAllData
#define SetPar(Name) m##Name = a##Name; m##Name##Orig = a##Name;
#define SetParRO(Name) m##Name = a##Name;


// for use in save for construct update strings and values
#define CheckParChange(Name, FieldName) \
    if (m##Name != m##Name##Orig) { \
        lUpdStr += #FieldName"=?,"; \
        lUpdValues.append(m##Name); \
    }

#define CheckParChangeWithNull(Name, FieldName, NullValue) \
    if (m##Name != m##Name##Orig) { \
        lUpdStr += #FieldName"=?,"; \
        if (m##Name != NullValue) \
            lUpdValues.append(m##Name); \
        else \
            lUpdValues.append(QVariant()); \
    }

#define RollbackPar(Name) m##Name = m##Name##Orig;
#define CommitPar(Name) m##Name##Orig = m##Name;

#endif // COMMONDATA_H
