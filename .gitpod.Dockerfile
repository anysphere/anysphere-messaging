FROM gitpod/workspace-full

RUN sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
RUN sudo apt-get update && sudo apt-get install -y build-essential cmake valgrind doxygen graphviz ccache cppcheck

RUN brew install bazel

RUN echo 'export CC=clang' >> /home/gitpod/.bashrc
#RUN echo 'export CC=gcc' >> /home/gitpod/.bashrc

RUN echo 'export CCX=clang++' >> /home/gitpod/.bashrc
#RUN echo 'export CCX=g++' >> /home/gitpod/.bashrc

RUN git config --global alias.c commit
RUN git config --global alias.s commit
RUN git config --global alias.p commit
RUN git config --global pull.ff only