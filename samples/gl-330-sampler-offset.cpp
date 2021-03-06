//**********************************
// OpenGL Sampler offset
// 18/02/2011
//**********************************
// Christophe Riccio
// ogl-samples@g-truc.net
//**********************************


#include <glf/glf.hpp>

namespace
{
	std::string const SAMPLE_NAME = "OpenGL Sampler offset";
	std::string const TEXTURE_DIFFUSE(glf::DATA_DIRECTORY + "kueken1-bgr8.dds");
	int const SAMPLE_SIZE_WIDTH(640);
	int const SAMPLE_SIZE_HEIGHT(480);
	int const SAMPLE_MAJOR_VERSION(3);
	int const SAMPLE_MINOR_VERSION(3);

	glf::window Window(glm::ivec2(SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT));

	GLsizei const VertexCount = 4;
	GLsizeiptr const VertexSize = VertexCount * sizeof(glf::vertex_v2fv2f);
	glf::vertex_v2fv2f const VertexData[VertexCount] =
	{
		glf::vertex_v2fv2f(glm::vec2(-1.0f,-1.0f), glm::vec2(0.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f,-1.0f), glm::vec2(1.0f, 1.0f)),
		glf::vertex_v2fv2f(glm::vec2( 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
		glf::vertex_v2fv2f(glm::vec2(-1.0f, 1.0f), glm::vec2(0.0f, 0.0f))
	};

	GLsizei const ElementCount = 6;
	GLsizeiptr const ElementSize = ElementCount * sizeof(GLuint);
	GLuint const ElementData[ElementCount] =
	{
		0, 1, 2, 
		2, 3, 0
	};

	namespace buffer
	{
		enum type
		{
			VERTEX,
			ELEMENT,
			MAX
		};
	}//namespace buffer

	namespace program
	{
		enum type
		{
			OFFSET,
			LINEAR,
			MAX
		};
	}//namespace program

	std::string const SHADER_VERT(glf::DATA_DIRECTORY + "gl-330/sampler.vert");

	std::string const SHADER_FRAG[program::MAX] = 
	{
		glf::DATA_DIRECTORY + "gl-330/sampler-offset.frag",
		glf::DATA_DIRECTORY + "gl-330/sampler-bicubic.frag",
	};

	GLuint VertexArrayName(0);
	GLuint ProgramName[program::MAX];
	GLint UniformMVP[program::MAX];
	GLint UniformDiffuse[program::MAX];
	GLint UniformOffset;
	glm::vec4 Viewport[program::MAX];

	GLuint BufferName[buffer::MAX];
	GLuint Image2DName(0);
}//namespace

bool initDebugOutput()
{
	bool Validated(true);

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallbackARB(&glf::debugOutput, NULL);

	return Validated;
}

bool initProgram()
{
	bool Validated = true;

	for(int i = 0; i < program::MAX; ++i)
	{
		GLuint VertShaderName = glf::createShader(GL_VERTEX_SHADER, SHADER_VERT);
		GLuint FragShaderName = glf::createShader(GL_FRAGMENT_SHADER, SHADER_FRAG[i]);

		ProgramName[i] = glCreateProgram();
		glAttachShader(ProgramName[i], VertShaderName);
		glAttachShader(ProgramName[i], FragShaderName);
		glDeleteShader(VertShaderName);
		glDeleteShader(FragShaderName);
		glLinkProgram(ProgramName[i]);
		Validated = Validated && glf::checkProgram(ProgramName[i]);

		UniformMVP[i] = glGetUniformLocation(ProgramName[i], "MVP");
		UniformDiffuse[i] = glGetUniformLocation(ProgramName[i], "Diffuse");
	}

	UniformOffset = glGetUniformLocation(ProgramName[program::OFFSET], "Offset");

	return Validated && glf::checkError("initProgram");
}

bool initVertexBuffer()
{
	glGenBuffers(buffer::MAX, BufferName);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ElementSize, ElementData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, VertexSize, VertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return glf::checkError("initArrayBuffer");
}

bool initTexture2D()
{
	glGenTextures(1, &Image2DName);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Image2DName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

	gli::texture2D Image = gli::load(TEXTURE_DIFFUSE);
	for(std::size_t Level = 0; Level < Image.levels(); ++Level)
	{
		glTexImage2D(
			GL_TEXTURE_2D, 
			GLint(Level), 
			GL_RGB, 
			GLsizei(Image[Level].dimensions().x), 
			GLsizei(Image[Level].dimensions().y), 
			0,  
			GL_BGR, 
			GL_UNSIGNED_BYTE, 
			Image[Level].data());
	}

	return glf::checkError("initTexture2D");
}

bool initVertexArray()
{
	glGenVertexArrays(1, &VertexArrayName);
    glBindVertexArray(VertexArrayName);
		glBindBuffer(GL_ARRAY_BUFFER, BufferName[buffer::VERTEX]);
		glVertexAttribPointer(glf::semantic::attr::POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(0));
		glVertexAttribPointer(glf::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(glf::vertex_v2fv2f), GLF_BUFFER_OFFSET(sizeof(glm::vec2)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnableVertexAttribArray(glf::semantic::attr::POSITION);
		glEnableVertexAttribArray(glf::semantic::attr::TEXCOORD);
	glBindVertexArray(0);

	return glf::checkError("initVertexArray");
}

bool begin()
{
	bool Validated = glf::checkGLVersion(SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);

	int const Border(1);
	glm::ivec2 const ViewportSize = Window.Size / 2 - Border * 2;

	Viewport[program::OFFSET] = glm::vec4(Border, Border, Window.Size.x / 2 - Border * 2, Window.Size.y - Border * 2);
	Viewport[program::LINEAR] = glm::vec4(Border + Window.Size.x / 2, Border, Window.Size.x / 2 - Border * 2, Window.Size.y - Border * 2);

	if(Validated && glf::checkExtension("GL_ARB_debug_output"))
		Validated = initDebugOutput();
	if(Validated)
		Validated = initProgram();
	if(Validated)
		Validated = initVertexBuffer();
	if(Validated)
		Validated = initVertexArray();
	if(Validated)
		Validated = initTexture2D();

	return Validated && glf::checkError("begin");
}

bool end()
{
	glDeleteBuffers(buffer::MAX, BufferName);
	for(int i = 0; i < program::MAX; ++i)
		glDeleteProgram(ProgramName[i]);
	glDeleteTextures(1, &Image2DName);
	glDeleteVertexArrays(1, &VertexArrayName);

	return glf::checkError("end");
}

void display()
{
	// Compute the MVP (Model View Projection matrix)
	glm::mat4 Projection = glm::perspective(45.0f, 2.0f / 3.0f, 0.1f, 1000.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Window.TranlationCurrent.y));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, Window.RotationCurrent.y, glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 View = glm::rotate(ViewRotateX, Window.RotationCurrent.x, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 MVP = Projection * View * Model;

	glViewport(0, 0, Window.Size.x, Window.Size.y);
	glClearBufferfv(GL_COLOR, 0, &glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)[0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Image2DName);

	glBindVertexArray(VertexArrayName);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferName[buffer::ELEMENT]);

	glUseProgram(ProgramName[program::OFFSET]);
	glUniform2iv(UniformOffset, 1, &glm::ivec2(63, 107)[0]);

	for(std::size_t i = 0; i < program::MAX; ++i)
	{
		glUseProgram(ProgramName[i]);
		glUniformMatrix4fv(UniformMVP[i], 1, GL_FALSE, &MVP[0][0]);
		glUniform1i(UniformDiffuse[i], 0);

		glViewportIndexedfv(0, &glm::vec4(Viewport[i])[0]);

		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, NULL, 1, 0);
	}

	glf::checkError("display");
	glf::swapBuffers();
}

int main(int argc, char* argv[])
{
	return glf::run(
		argc, argv,
		glm::ivec2(::SAMPLE_SIZE_WIDTH, ::SAMPLE_SIZE_HEIGHT), 
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, ::SAMPLE_MAJOR_VERSION, 
		::SAMPLE_MINOR_VERSION);
}
