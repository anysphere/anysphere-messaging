export function BackgroundCircles(props: { backgroundColor?: string }) {
  console.log(props);

  return (
    <div className={`flex min-h-screen items-center justify-center`}>
      <div className="relative w-full max-w-lg">
        <div
          className={`animation-delay-2000 absolute top-[20px] left-[135px] h-[115px] w-[115px] animate-blob rounded-full bg-gradient-to-t 
                      from-asbrown-dark to-asbrown-dark/70 opacity-80 mix-blend-multiply shadow-lg shadow-stone-800 blur-lg filter
                      md:top-[10px] md:left-[-10px] md:h-[230px] md:w-[230px] `}
        ></div>
        <div
          className="absolute -top-[50px] left-[200px] h-[130px] w-[130px] animate-blob rounded-full bg-gradient-to-t from-asyellow-dark 
                      to-asyellow-light/70 opacity-80 mix-blend-multiply shadow-lg shadow-stone-800 blur-lg filter 
                      md:-top-[130px] md:left-[260px] md:h-[300px] md:w-[300px]"
        ></div>
        <div
          className="animation-delay-4000 absolute -bottom-4 left-10 h-[190px] w-[190px] animate-blob rounded-full bg-gradient-to-t from-asgreen-dark
                      to-asgreen-dark/70 opacity-90 mix-blend-multiply shadow-lg shadow-stone-800 blur-lg filter 
                      md:-bottom-3 md:left-0 md:h-[440px] md:w-[440px]"
        ></div>
      </div>
    </div>
  );
}
