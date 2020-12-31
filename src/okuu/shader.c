#include "shader.h"

shader_t* _active = NULL;

shader_t* shader_create(const char* name) {
    shader_t* shader = malloc(sizeof(shader_t));
    shader->name = name;
    shader->layout = NULL;
    shader->layout_cnt = 0;
    shader->attribs = NULL;
    shader->program = glCreateProgram();
    shader->loaded = 0;

    return shader;
}

int shader_layout(shader_t* shader, int count, ...) {
    if(shader->loaded)
        return -1;
    if(shader->layout != NULL)
        free(shader->layout);
    shader->layout = malloc(sizeof(layout_name_t) * count);
    shader->layout_cnt = count;

    va_list args;
    va_start(args, count);
    for(int i = 0; i < count; ++i)
        strcpy(shader->layout[i], va_arg(args, const char*));
    va_end(args);

    return 0;
}

int shader_source(shader_t* shader, int is_src, int files, ...) {
    if(shader->loaded)
        return -1;

    int failed = 0;
    GLuint* shaders = calloc(sizeof(GLuint), files);
    GLint err;

    va_list args;
    va_start(args, files);
    for(int i = 0; i < files; ++i) {
        char* file_name = "INLINE";
        char* source = NULL;

        if(!is_src) {
            file_name = va_arg(args, char*);
            source = file_read(file_name);
            if(source == NULL) {
                failed = 1;
                break;
            }
        } else
            source = va_arg(args, char*);

        shaders[i] = glCreateShader(va_arg(args, GLuint));
        if(shaders[i] == 0) {
            if(!is_src)
                free(source);
            failed = 1;
            break;
        }

        glShaderSource(shaders[i], 1, &source, NULL);
        glCompileShader(shaders[i]);

        glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &err);
        if(err == GL_FALSE) {
            glGetShaderiv(shaders[i], GL_INFO_LOG_LENGTH, &err);

            char* msg = malloc(sizeof(char) * err);
            glGetShaderInfoLog(shaders[i], err, NULL, msg);
            fprintf(__STDERR, "[ERROR] (%s (COMPILER)) %s\n", file_name, msg);
            free(msg);

            if(!is_src)
                free(source);
            failed = 1;
            break;
        }

        glAttachShader(shader->program, shaders[i]);
        if(!is_src)
            free(source);
    }
    va_end(args);

    for(int i = 0; i < files; ++i)
        glDeleteShader(shaders[i]);
    free(shaders);

    if(failed)
        return -1;

    if(shader->layout != NULL) {
        for(int i = 0; i < shader->layout_cnt; ++i)
            glBindAttribLocation(shader->program, i, shader->layout[i]);
        free(shader->layout);
    }

    glLinkProgram(shader->program);
    glGetProgramiv(shader->program, GL_LINK_STATUS, &err);
    if(err == GL_FALSE) {
        glGetProgramiv(shader->program, GL_INFO_LOG_LENGTH, &err);

        char* msg = malloc(sizeof(char) * err);
        glGetProgramInfoLog(shader->program, err, NULL, msg);
        fprintf(__STDERR, "[ERROR] (%s (LINKER)) %s\n", shader->name, msg);
        free(msg);

        return -1;
    }

    shader->loaded = 1;
    return 0;
}

int shader_attribs(shader_t* shader, int count, ...) {
    if(!shader->loaded)
        return -1;
    if(shader->attribs == NULL)
        free(shader->attribs);

    shader->attribs = malloc(sizeof(GLint) * count);
    va_list args;
    va_start(args, count);
    for(int i = 0; i < count; ++i) {
        const char* attr = va_arg(args, const char*);

        if((shader->attribs[i] =
            glGetUniformLocation(shader->program, attr)) == -1)
        {
            fprintf(__STDERR, "[WARN] (%s) Could not find uniform '%s'\n",
                shader->name, attr);

            // TODO determine if this is necessary, probably not
            /*free(shader->attribs);
            shader->attribs = NULL;
            return -1;*/
        }
    }
    va_end(args);

    return 0;
}

GLint shader_attr(const shader_t* shader, int i) {
    if(_SHADER == NULL)
        return -1;

    return shader->attribs[i];
}

shader_t* shader_active() {
    return _active;
}

void shader_start(shader_t* shader) {
    if(_active == NULL || _active->program != shader->program) {
        glUseProgram(shader->program);
        _active = shader;
    }
}

void shader_stop() {
    glUseProgram(0);
    _active = NULL;
}

void shader_destroy(shader_t* shader) {
    free(shader->attribs);
    glDeleteProgram(shader->program);
    shader->loaded = 0;
}