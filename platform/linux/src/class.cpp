#include <platform.h>
#include <class.h>
#include <list>
#include <map>
#include <string>
#include <atomic>
#include <mutex>

#include <gdi.h>

static std::recursive_mutex cls_mutex;
static std::list<CLASS*> class_list;
static std::map<std::string,ATOM> atom_map;
static std::atomic_uint32_t atom_start(10);

/***********************************************************************
 *           get_int_atom_value
 */
ATOM get_int_atom_value( const char *name )
{
    const char *ptr = name;
    const char *end = ptr + strlen(name);
    UINT ret = 0;

    if (IS_INTRESOURCE(ptr)) return LOWORD(ptr);

    if (*ptr++ != '#') return 0;
    while (ptr < end)
    {
        if (*ptr < '0' || *ptr > '9') return 0;
        ret = ret * 10 + *ptr++ - '0';
        if (ret > 0xffff) return 0;
    }
    return ret;
}

BOOL GetAtomName( ATOM atomName, LPSTR name, int cchLen)
{
    std::unique_lock<std::recursive_mutex> lock(cls_mutex);
    for(auto & it:atom_map){
        if(it.second == atomName)
        {
            if(cchLen >it.first.length())
                strcpy(name,it.first.c_str());
            return TRUE;
        }
    }
    return FALSE;
}

/***********************************************************************
 *		RegisterClassExW (USER32.@)
 */
ATOM WINAPI RegisterClassEx( const WNDCLASSEX *wc)
{
    HINSTANCE instance;
    CLASS *_class;
    ATOM atom;
    BOOL ret;

    if (wc->cbSize != sizeof(*wc) || wc->cbClsExtra < 0 || wc->cbWndExtra < 0 )
    {
         SetLastError( ERROR_INVALID_PARAMETER );
         return 0;
    }
    if (!(instance = wc->hInstance))
    {
         SetLastError( ERROR_INVALID_PARAMETER );
         //todo:hjx
         //return 0;
    }

    /* Fix the extra bytes value */

    if (wc->cbClsExtra > 40)  /* Extra bytes are limited to 40 in Win32 */
        WARN( "Class extra bytes %d is > 40\n", wc->cbClsExtra);
    if (wc->cbWndExtra > 40)  /* Extra bytes are limited to 40 in Win32 */
        WARN("Win extra bytes %d is > 40\n", wc->cbWndExtra );

    if (!(_class = (CLASS*)calloc( 1, sizeof(CLASS) + wc->cbClsExtra ))) return 0;

    _class->atomName = get_int_atom_value( wc->lpszClassName );
    _class->basename = _class->name;
    if (!_class->atomName && wc->lpszClassName)
    {
        strcpy( _class->name, wc->lpszClassName);
    }
    else
    {
        GetAtomName( _class->atomName, _class->name,sizeof(_class->name));
    }

    _class->style      = wc->style;
    _class->cbWndExtra = wc->cbWndExtra;
    _class->cbClsExtra = wc->cbClsExtra;
    _class->instance   = (UINT_PTR)instance;

    /* Other non-null values must be set by caller */
    // if (wc->hIcon && !wc->hIconSm)
    //     sm_icon = CopyImage( wc->hIcon, IMAGE_ICON,
    //                          get_system_metrics( SM_CXSMICON ),
    //                          get_system_metrics( SM_CYSMICON ),
    //                          LR_COPYFROMRESOURCE );


    _class->hIcon         = wc->hIcon;
    _class->hIconSm       = wc->hIconSm;
    _class->hCursor       = wc->hCursor;
    _class->hbrBackground = wc->hbrBackground;
    _class->winproc       = wc->lpfnWndProc;
    if(!_class->atomName){
        //todo:hjx 
        std::unique_lock<std::recursive_mutex> lock(cls_mutex);
        auto it = atom_map.find(_class->name);
        if(it!=atom_map.end())
        {
            _class->atomName = it->second;
        }    
        else
        {
            _class->atomName = ++atom_start;
            atom_map.insert(std::make_pair(_class->name,_class->atomName));
        }    
    }else{
        std::unique_lock<std::recursive_mutex> lock(cls_mutex);
        atom_map.insert(std::make_pair(_class->name,_class->atomName));
    }
    atom = _class->atomName;
    {
        std::unique_lock<std::recursive_mutex> lock(cls_mutex);
        class_list.push_front(_class);
    }

    return atom;
}


/***********************************************************************
 *		UnregisterClassW (USER32.@)
 */
BOOL WINAPI UnregisterClass( LPCSTR className, HINSTANCE instance )
{
    std::unique_lock<std::recursive_mutex> lock(cls_mutex);
    for(auto it = class_list.begin();it!=class_list.end();it++){
        CLASS * _class = *it;
        BOOL bMatch = FALSE;
        if (IS_INTRESOURCE(className))
            bMatch = _class->atomName == (ATOM)(UINT_PTR)className;
        else
            bMatch = strcmp(_class->name, className) == 0;
        if(bMatch)
        {
            atom_map.erase(std::string(_class->name));
            //if (_class->hbrBackground > (HBRUSH)(COLOR_GRADIENTINACTIVECAPTION + 1))
            DeleteObject( _class->hbrBackground );
            free(_class);
            class_list.erase(it);
            return TRUE;
        }
    }
    return FALSE;
}

static CLASS *find_class( HINSTANCE module, LPCSTR clsName ){
    std::unique_lock<std::recursive_mutex> lock(cls_mutex);
    if(IS_INTRESOURCE(clsName)){
        ATOM atom = (ATOM)reinterpret_cast<LONG_PTR>(clsName);
        bool bValid = false;
        for(auto & it : atom_map){
            if(it.second  == atom){
                clsName = it.first.c_str();
                bValid = true;
                break;
            }
        }
        if(!bValid)
            return nullptr;
    }
    for(auto & it : class_list){
        if(strcmp(it->name, clsName)==0)
            return it;
    }
    return NULL;
}

static ATOM get_class_info( HINSTANCE instance, const char *class_name, WNDCLASSEX * wc)
{
    std::unique_lock<std::recursive_mutex> lock(cls_mutex);
    CLASS* _class;
    ATOM atom;
    if (!(_class = find_class(instance, class_name))) return 0;

    if (wc)
    {
        wc->style = _class->style;
        wc->lpfnWndProc = _class->winproc;
        wc->cbClsExtra = _class->cbClsExtra;
        wc->cbWndExtra = _class->cbWndExtra;
        wc->hInstance = instance;
        wc->hIcon = _class->hIcon;
        wc->hIconSm = _class->hIconSm;
        wc->hCursor = _class->hCursor;
        wc->hbrBackground = _class->hbrBackground;
        wc->lpszClassName = _class->basename;
    }
    atom = _class->atomName;
    return atom;
}

/***********************************************************************
 *		GetClassInfoExW (USER32.@)
 */
BOOL WINAPI GetClassInfoEx( HINSTANCE hInstance, LPCSTR name, WNDCLASSEX *wc )
{
    ATOM atom;

    if (!wc)
    {
        SetLastError( ERROR_NOACCESS );
        return FALSE;
    }

    atom = get_class_info( hInstance, name, wc );
    if (atom) wc->lpszClassName = name;

    /* We must return the atom of the class here instead of just TRUE. */
    return atom;
}
