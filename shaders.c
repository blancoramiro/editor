/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

#define build_fragment_shader(program) \
        fragment_shader_##program = glCreateShader(GL_FRAGMENT_SHADER); \
        glShaderSource(fragment_shader_##program, 1, &fragment_shader_##program##_code, NULL); \
        glCompileShader(fragment_shader_##program); \
        glGetShaderiv(fragment_shader_##program, GL_COMPILE_STATUS, &success); \
        if(GL_TRUE != success) \
        { \
                glGetShaderiv(fragment_shader_##program, GL_INFO_LOG_LENGTH, &success); \
                glGetShaderInfoLog(fragment_shader_##program, 512, NULL, infoLog); \
                print_to_screen("ERROR Fragment " #program " program: "); \
                print_to_screen(infoLog); \
                print_to_screen("\n"); \
                exit(1); \
        } \
        program_##program = glCreateProgram(); \
        glAttachShader(program_##program, vertex_shader); \
        glAttachShader(program_##program, fragment_shader_##program); \
        glLinkProgram(program_##program); \
        glGetProgramiv(program_##program, GL_LINK_STATUS, &success); \
        if(!success) \
        { \
                glGetProgramInfoLog(program_##program, 512, NULL, infoLog); \
                print_to_screen("ERROR program" #program ": "); \
                print_to_screen(infoLog); \
                print_to_screen("\n"); \
                exit(1); \
        } \
        glUseProgram(program_##program); \
        vpos_location_##program = glGetAttribLocation(program_##program, "vPos"); \
        glEnableVertexAttribArray(vpos_location_##program);

const char* vertex_shader_text_code =
	"attribute vec2 vPos;"
	"varying vec2 tPos;"
	"void main(void)"
	"{"
	"    gl_Position = vec4(vPos, 0.0, 1.0);"
	"    tPos = vPos;"
	"}";


const char* fragment_shader_text_code =
#ifdef __EMSCRIPTEN__
	"precision highp float;"
#endif
	"varying vec2 tPos;"
	"uniform vec2 Mouse;"
	"uniform vec2 Cursor;"
	"uniform float CursorBlink;"
	"uniform vec4 Grid;"
	"uniform float GridYOffset;"
	"uniform float GridXOffset;"
	"uniform sampler2D Chars;"
	"uniform sampler2D Selection;"
	"uniform sampler2D Texture;"
	"float Char, Select;"
	"float xpos, ypos;"
	"vec4 Result;"
	"vec4 CharTex;"
	"void main(void)"
	"{"
#ifdef CHAR_GRID
	"	if(mod(ceil(gl_FragCoord.x), Grid.z) == 0. || mod(ceil(gl_FragCoord.y+GridYOffset), Grid.w) == 1.)"
	"	{"
	"		gl_FragColor = vec4(1., 1., 1., 0.);"
	"	}"
	"	else"
	"	{"
#endif
	"		xpos = ceil((gl_FragCoord.x-GridXOffset)*(1./Grid.z));"
	"		ypos = Grid.y-floor((gl_FragCoord.y+GridYOffset)*(1./Grid.w));"
	"		CharTex = texture2D(Chars, vec2((1./Grid.x*xpos-1./Grid.x/2.)   , 1./Grid.y*(.5+ypos)));"
	"		Char = CharTex.a*256.;"
	"		Select = texture2D(Selection, vec2((1./Grid.x*xpos-1./Grid.x/2.)   , 1./Grid.y*(.5+ypos))).r*256.;"
	"		Result = ceil(texture2D(Texture, vec2(((mod(gl_FragCoord.x-GridXOffset, Grid.z)*14.)/Grid.z)/256.+(14./256.)*ceil(mod(Char,18.)), ((mod(gl_FragCoord.y+GridYOffset, Grid.w)*18.)/Grid.w)/128.+(18./128.)*  ceil(6.-Char/18.)     )));"
	"		if((ypos == Cursor.y && xpos == Cursor.x && CursorBlink > 0.) || ypos == Mouse.y && xpos == Mouse.x)"
	"		gl_FragColor = vec4(1., 0., 0., 0.) - Result;"
	"		else gl_FragColor = (1. - Select) * (CharTex * Result) + Select * (CharTex * (1. - Result));"
#ifdef CHAR_GRID
	"	}"
#endif
	"}";

const char* fragment_shader_scroll_code =
#ifdef __EMSCRIPTEN__
	"precision highp float;"
#endif
	"uniform vec2 ScrollBar;"
	"void main(void)"
	"{"
	"      gl_FragColor = vec4(.08, .08, .08, .1) + vec4(.3, 0., 0., 0.) * vec4(ceil(clamp(gl_FragCoord.y - ScrollBar.y, 0., 1.)) - ceil(clamp(gl_FragCoord.y - ScrollBar.x, 0., 1.)));"
	"}";

const char* fragment_shader_linenumber_code =
#ifdef __EMSCRIPTEN__
	"precision highp float;"
#endif
	"uniform sampler2D Texture;"
	"uniform sampler2D LineNumbers;"
	"uniform vec4 Grid;"
	"uniform float GridYOffset;"
	"float xpos, ypos, Char;"
	"void main(void)"
	"{"
#ifdef CHAR_GRID
	"	if(mod(ceil(gl_FragCoord.x), Grid.z) == 0. || mod(ceil(gl_FragCoord.y+GridYOffset), Grid.w) == 1.)"
	"	{"
	"		gl_FragColor = vec4(1., 1., 1., 0.);"
	"	}"
	"	else"
	"	{"
#endif
	"		xpos = ceil(gl_FragCoord.x*(1./Grid.z));"
	"		ypos = Grid.y-floor((gl_FragCoord.y+GridYOffset)*(1./Grid.w));"
	"		Char = texture2D(LineNumbers, vec2((1./Grid.x*xpos-1./Grid.x/2.)   , 1./Grid.y*(.5+ypos))).r*256.;"
	//"		Char = texture2D(LineNumbers, vec2(1./1.*xpos-1./1./2., 1./Grid.y*(.5+ypos))).r*256.;"
	//"		Char = texture2D(LineNumbers, vec2((1./(Grid.z*3.)*xpos-1./(Grid.z*3.)/2.), 1./Grid.y*(.5+ypos))).r*256.;"
	//"		gl_FragColor = ceil(texture2D(Texture, vec2(((mod(gl_FragCoord.x, Grid.z)*14.)/Grid.z)/256.+(14./256.)*ceil(mod(Char,18.)), ((mod(gl_FragCoord.y+GridYOffset, Grid.w)*18.)/Grid.w)/128.+(18./128.)*ceil(6.-Char/18.))));"
	"		if(ypos == 3.)"
	"		gl_FragColor = vec4(0.,1., 1., 0.) * ceil(texture2D(Texture, vec2(((mod(gl_FragCoord.x, Grid.z)*14.)/Grid.z)/256.+(14./256.)*ceil(mod(Char,18.)), ((mod(gl_FragCoord.y+GridYOffset, Grid.w)*18.)/Grid.w)/128.+(18./128.)*  ceil(6.-Char/18.)     )));"
	"		else"
	"		gl_FragColor = vec4(1.,0., 1., 0.) * ceil(texture2D(Texture, vec2(((mod(gl_FragCoord.x, Grid.z)*14.)/Grid.z)/256.+(14./256.)*ceil(mod(Char,18.)), ((mod(gl_FragCoord.y+GridYOffset, Grid.w)*18.)/Grid.w)/128.+(18./128.)*  ceil(6.-Char/18.)     )));"
#ifdef CHAR_GRID
	"	}"
#endif
	"}";

