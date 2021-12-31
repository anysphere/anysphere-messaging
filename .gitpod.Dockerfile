FROM gitpod/workspace-full

RUN sudo bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
RUN sudo apt-get update && sudo apt-get install -y build-essential cmake valgrind doxygen graphviz ccache cppcheck

RUN sudo apt install apt-transport-https curl gnupg
RUN curl -fsSL https://bazel.build/bazel-release.pub.gpg | gpg --dearmor > bazel.gpg
RUN sudo mv bazel.gpg /etc/apt/trusted.gpg.d/
RUN echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list

RUN sudo apt update && sudo apt install bazel
RUN sudo apt install openjdk-11-jdk

RUN echo 'export CC=clang' >> /home/gitpod/.bashrc
#RUN echo 'export CC=gcc' >> /home/gitpod/.bashrc

RUN echo 'export CCX=clang++' >> /home/gitpod/.bashrc
#RUN echo 'export CCX=g++' >> /home/gitpod/.bashrc
