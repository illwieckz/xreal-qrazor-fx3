import os

Import('env')

#env.Jar(target = 'game.jar', source = ['com', 'javax', 'xreal'])
#os.system('rm ../game.jar')
#os.remove('../game.jar')
os.system('zip ../game.jar -r META-INF com javax xreal -x "*.svn*" -x "*.java" ')

