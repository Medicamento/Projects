package implementor;

import info.kgeorgiy.java.advanced.implementor.ImplerException;
import info.kgeorgiy.java.advanced.implementor.JarImpler;

import javax.tools.JavaCompiler;
import javax.tools.ToolProvider;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Parameter;
import java.net.URISyntaxException;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.jar.JarEntry;
import java.util.jar.JarOutputStream;
import java.util.jar.Manifest;
import java.util.stream.Collectors;

/**
 * This class is designed to automatically generate implementations of Java interfaces, compile it, and package the compiled class into a JAR file.
 * Implements the {@link JarImpler} interface.
 *
 * @see JarImpler
 */
public class Implementor implements JarImpler {
    /**
     * System-dependent line separator.
     */
    private static final String LINE_SEPARATOR = System.lineSeparator();
    /**
     * Tabulation.
     */
    private static final String TABULATION = "\t";
    /**
     * Space.
     */
    private static final String SPACE = " ";
    /**
     * StringBuilder for storing the result of generation before writing to a file.
     */
    private static StringBuilder sb;

    /**
     * Writes the package statement for the specified interface.
     *
     * @param token type token to create implementation for.
     */
    private void writePackage(Class<?> token) {
        String packageName = token.getPackageName();
        if (packageName.isEmpty()) {
            return;
        }
        sb.append("package").append(SPACE).append(packageName).append(";").append(LINE_SEPARATOR);
        sb.append(LINE_SEPARATOR);
    }

    /**
     * Transforms a string into a Unicode sequence.
     *
     * @param name the string to encode
     * @return the encoded string
     */
    public static String encode(String name) {
        return name.chars().mapToObj(c -> c > 127 ? String.format("\\u%04x", c) :
                String.valueOf((char) c)).collect(Collectors.joining());
    }

    /**
     * Writes the class declaration for a given interface.
     *
     * @param token  type token to create implementation for.
     */
    private void writeClassDeclaration(Class<?> token) {
        sb.append("public").append(SPACE).append("class").append(SPACE).append(encode(token.getSimpleName()));
        sb.append("Impl").append(SPACE).append("implements").append(SPACE).append(encode(token.getCanonicalName()));
        sb.append(SPACE).append("{");
        sb.append(LINE_SEPARATOR);
    }

    /**
     * Generates a default return value for a required return type.
     *
     * @param returnVal the return type
     * @return the default return value
     */
    private Object getReturnVal(Class<?> returnVal) {
        if (!returnVal.isPrimitive()) {
            return null;
        }
        if (returnVal.equals(boolean.class)) {
            return false;
        } else {
            return 0;
        }
    }

    /**
     * Writes one method implementation.
     *
     * @param method method to create implementation for.
     */
    private void writeMethod(Method method) {
        int modifiers = method.getModifiers() & ~Modifier.ABSTRACT & ~Modifier.TRANSIENT;
        if (Modifier.isStatic(modifiers)) {
            return;
        }
        sb.append(TABULATION).append("@Override").append(LINE_SEPARATOR);
        sb.append(TABULATION).append(Modifier.toString(modifiers)).append(SPACE);

        String returnType = method.getReturnType().getCanonicalName();
        sb.append(returnType).append(SPACE);
        sb.append(method.getName()).append("(");

        boolean first = true;
        for (Parameter param : method.getParameters()) {
            if (first) {
                first = false;
            } else {
                sb.append(",").append(SPACE);
            }
            sb.append(param.getType().getCanonicalName()).append(SPACE).append(param.getName());
        }
        sb.append(")").append(SPACE).append("{").append(LINE_SEPARATOR);

        if (!method.getReturnType().equals(void.class)) {
            sb.append(TABULATION).append(TABULATION);
            sb.append("return").append(SPACE).append(getReturnVal(method.getReturnType())).append(";").append(LINE_SEPARATOR);
        }
        sb.append(TABULATION).append("}").append(LINE_SEPARATOR);
        sb.append(LINE_SEPARATOR);
    }

    /**
     * Writes implementations for all methods of a given interface.
     *
     * @param token type token to create implementation for.
     */
    private void writeMethods(Class<?> token) {
        for (var method : token.getMethods()) {
            writeMethod(method);
        }
    }

    /**
     * Generates the class definition to implement the given interface.
     *
     * @param token type token to create implementation for.
     */
    private void writeClass(Class<?> token) {
        writePackage(token);
        writeClassDeclaration(token);
        writeMethods(token);
        if (sb.substring(sb.length() - LINE_SEPARATOR.length()).equals(LINE_SEPARATOR))
            sb.delete(sb.length() - LINE_SEPARATOR.length(), sb.length());
        sb.append("}");
    }

    /**
     * Writes the entire class to file.
     *
     * @param token type token to create implementation for.
     * @param root root directory to write the file to.
     * @throws ImplerException when error occurs during writing
     */
    private void writeToFile(Class<?> token, Path root) throws ImplerException {
        Path packagePath = Path.of(token.getPackageName().replace('.', File.separatorChar));
        Path fullPath = root.resolve(packagePath);
        try {
            Files.createDirectories(fullPath);
            Path filePath = fullPath.resolve(token.getSimpleName() + "Impl.java");
            Files.writeString(filePath, sb.toString(), StandardCharsets.UTF_8);
        } catch (FileAlreadyExistsException e) {
            throw new ImplerException("File already exists: " + e.getFile());
        } catch (IOException e) {
            throw new ImplerException("Error while creating or writing to file");
        }
    }

    /**
     * Validates the provided class token. Checks if the token is an interface and isn't private.
     *
     * @param token type token to validate.
     * @throws ImplerException when the token is not suitable.
     */
    private void checkToken(Class<?> token) throws ImplerException {
        if (!token.isInterface()) {
            throw new ImplerException("Token should be an interface");
        }
        if (Modifier.isPrivate(token.getModifiers())) {
            throw new ImplerException("Required interface is private");
        }
    }

    /**
     * Returns the classpath for the specified class token.
     *
     * @param token type token to get path of.
     * @return the classpath as a String
     * @throws ImplerException when error occurs while getting the classpath
     */
    private static String getClassPath(Class<?> token) throws ImplerException {
        try {
            return Path.of(token.getProtectionDomain().getCodeSource().getLocation().toURI()).toString();
        } catch (final URISyntaxException e) {
            throw new ImplerException("Error while getting class path");
        }
    }

    /**
     * Generates the path and name for the implementation class to be generated.
     *
     * @param dir   the directory where the class is going to be created
     * @param token type token to create implementation for.
     * @return a string representation of the path and name for the implementation class
     */
    private static String getName(Path dir, Class<?> token) {
        return (dir.resolve(token.getPackageName().replace('.', File.separatorChar)
                + File.separator + token.getSimpleName() + "Impl")).toString();
    }

    /**
     * Compiles the generated class file.
     *
     * @param token type token to create implementation for.
     * @param root  root directory.
     * @throws ImplerException when compilation error occurs
     */
    private static void compile(Class<?> token, final Path root) throws ImplerException {
        final JavaCompiler compiler = ToolProvider.getSystemJavaCompiler();
        if (compiler == null) {
            throw new ImplerException("Could not find java compiler, include tools.jar to classpath");
        }
        final String classpath = root + File.pathSeparator + getClassPath(token);
        final int exitCode = compiler.run(null, null, null, "-encoding", "UTF-8", "-cp",
                classpath, getName(root, token) + ".java");
        if (exitCode != 0) {
            throw new ImplerException("Error while compiling " + exitCode);
        }
    }

    /**
     * {@inheritDoc}
     *
     *  @param token type token to create implementation for.
     *  @param root root directory.
     *  @throws ImplerException when implementation cannot be generated.
     */
    @Override
    public void implement(Class<?> token, Path root) throws ImplerException {
        checkToken(token);
        sb = new StringBuilder();
        writeClass(token);
        writeToFile(token, root);
    }

    /**
     * {@inheritDoc}
     *
     * @param token  type token to create implementation for.
     * @param jarFile target <var>.jar</var> file.
     * @throws ImplerException when implementation cannot be generated.
     */
    @Override
    public void implementJar(Class<?> token, Path jarFile) throws ImplerException {
        Path root = Paths.get("");
        implement(token, root);
        compile(token, root);
        createJarFile(token, root, jarFile);
    }

    /**
     * Creates a JAR file containing the compiled implementation of the required interface.
     *
     * @param token   the class token of the implemented interface
     * @param root    the root directory containing the compiled files
     * @param jarFile the path to the JAR file to be created
     * @throws ImplerException if an error occurs while creating the JAR file
     */
    private void createJarFile(Class<?> token, Path root, Path jarFile) throws ImplerException {
        Manifest manifest = new Manifest();
        try (JarOutputStream jarOutputStream = new JarOutputStream(Files.newOutputStream(jarFile), manifest)) {
            String classPath = getName(root, token).replace(File.separator, "/") + ".class";
            jarOutputStream.putNextEntry(new JarEntry(classPath));
            Files.copy(root.resolve(classPath), jarOutputStream);
            jarOutputStream.closeEntry();
        } catch (IOException e) {
            throw new ImplerException("Error while creating jar file");
        }
    }

    /**
     * The main method, which processes command-line arguments to generate implementation of
     * classes or also package them to .jar files.
     *
     * <p>Usage:</p>
     * <ul>
     *     <li>
     *         With two arguments: {@code <class-name> <output-path>}
     *         <p>Generates the implementation of the interface specified by {@code class-name} and saves it to the file specified by {@code output-path}.</p>
     *     </li>
     *     <li>
     *         With three arguments: {@code -jar <class-name> <jar-file>}
     *         <p>Generates the implementation of the interface specified by {@code class-name}, compiles it, and packages the compiled class into a JAR file specified by {@code jar-file}.</p>
     *     </li>
     * </ul>
     * @param args command-line arguments
     */
    public static void main(String[] args) {
        if (args == null || (args.length != 2 && args.length != 3)) {
            System.err.println("Usage: (-jar) <class-name> <file.jar>");
            return;
        }
        for (String arg : args) {
            if (arg == null) {
                System.err.println("Argument shouldn't be null");
            }
        }
        Path root = Paths.get("").toAbsolutePath(), path;
        Class<?> token;
        Implementor implementor = new Implementor();
        try {
            if (args.length == 2) {
                token = Class.forName(args[0]);
                path = root.resolve(args[1]);
                implementor.implement(token, path);
            }
            else if ("-jar".equals(args[0])) {
                token = Class.forName(args[1]);
                path = root.resolve(args[2]);
                implementor.implementJar(token, path);
            } else {
                System.err.println("Usage: -jar <class-name> <file.jar>");
            }
        } catch (ClassNotFoundException e) {
            System.err.println("No such token");
        } catch (ImplerException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
