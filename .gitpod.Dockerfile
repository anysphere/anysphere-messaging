FROM gitpod/workspace-full

RUN sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
RUN sudo apt-get update && sudo apt-get install -y build-essential cmake valgrind doxygen graphviz ccache cppcheck libpq-dev

RUN brew install bazel && brew install buildifier && brew install grpcurl
 
# RUN go install github.com/fullstorydev/grpcui/cmd/grpcui@latest

RUN echo 'export CC=clang' >> /home/gitpod/.bashrc
#RUN echo 'export CC=gcc' >> /home/gitpod/.bashrc

RUN echo 'export CCX=clang++' >> /home/gitpod/.bashrc
#RUN echo 'export CCX=g++' >> /home/gitpod/.bashrc

RUN git config --global alias.c commit && git config --global alias.s status && git config --global alias.p push && git config --global pull.ff only

RUN brew install fzf && $(brew --prefix)/opt/fzf/install