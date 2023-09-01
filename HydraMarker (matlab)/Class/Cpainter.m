classdef Cpainter < handle
    % paint markers based on marker field

    methods (Static)
        function img = draw_bw(state,L)
            %mustBeA(state,'Dfield');
            %mustBeInteger(L);
            assert(L>=5,'side length must be no less than 5!');
            
            img = state.val;
            img(img==-1) = 0.66;
            img(img==-2) = 0.33;
            img = imresize(img,size(img)*L,'nearest');
        end
        
        function img = draw_chessboard(state,L)
            %mustBeA(state,'Dfield');
            %mustBeInteger(L);
            assert(L>=5,'side length must be no less than 5!');
            
            state.val = padarray(state.val,[1 1],-2,'both');
            mask = state.val>=0;
            mask_d = imdilate(mask,strel('square',3));
            state.val(mask_d & ~mask) = -3;
            
            [M,N] = size(state.val);
            
            % draw the background
            row = ones(M,1);    col = ones(1,N);
            row(1:2:end) = -1;  col(1:2:end) = -1;
            img = row*col;
            img(state.val==-3) = 0.05*img(state.val==-3)+0.5;
            img(img==-1)=0;
            img(state.val==-2) = 0.5;
            
            img = imresize(img,size(img)*L,'nearest');

            % draw the foreground
            circle_x = (1+L)/2:L:size(img,2);
            circle_x = repmat(circle_x,[M,1]);
            circle_x = circle_x(:);
            circle_y = (1+L)/2:L:size(img,1);
            circle_y = repmat(circle_y,[1,N]);
            circle_y = circle_y(:);

            circle_matrix = [circle_x,circle_y,repmat(ceil(L/6),[size(circle_x,1),1])];
            img = insertShape(img,'FilledCircle',circle_matrix(state.val(:)==1,:),'Color','white','Opacity',1);

            % if the background style is chessboard, some dots should be black.
            b_square = ~xor(rem(1:N,2),rem((1:M)',2));
            img = insertShape(img,'FilledCircle',circle_matrix(state.val(:)==1 & b_square(:),:),'Color','black','Opacity',1);

            % reduce the width of pad
            % img = padarray(img,[ceil(L/2) ceil(L/2)],'symmetric','both');

        end % end function
        
        function img = draw_crossfield(state,L)
            img = imresize(state.val,2,'nearest');
            [im,in] = meshgrid(1:size(img,1),1:size(img,2));
            im = rem(im,2);
            in = rem(in,2);
            img = xor(img,-xor(im,in));
            img = imresize(img,L/2,'nearest');
        end
        
        function img = draw_dots(state,L)
            img = imresize(ones(size(state.val)),L,'nearest');
            [M,N] = size(state.val);
            
            % draw the circles
            circle_x = (1+L)/2:L:size(img,2);
            circle_x = repmat(circle_x,[M,1]);
            circle_x = circle_x(:);
            circle_y = (1+L)/2:L:size(img,1);
            circle_y = repmat(circle_y,[1,N]);
            circle_y = circle_y(:);

            r = ceil(L/12);
            circle_matrix = [circle_x,circle_y,repmat(r,[size(circle_x,1),1])];
            img = insertShape(img,'FilledCircle',circle_matrix(state.val(:)==0,:),'Color','black','Opacity',1);
            
            r = ceil(L/24);
            circle_matrix = [circle_x-1.5*r,circle_y-1.5*sqrt(3)*r,repmat(r,[size(circle_x,1),1])];
            img = insertShape(img,'FilledCircle',circle_matrix(state.val(:)==1,:),'Color','black','Opacity',1);
            circle_matrix = [circle_x-1.5*r,circle_y+1.5*sqrt(3)*r,repmat(r,[size(circle_x,1),1])];
            img = insertShape(img,'FilledCircle',circle_matrix(state.val(:)==1,:),'Color','black','Opacity',1);
            circle_matrix = [circle_x+1.5*2*r,circle_y,repmat(r,[size(circle_x,1),1])];
            img = insertShape(img,'FilledCircle',circle_matrix(state.val(:)==1,:),'Color','black','Opacity',1);
        end
        
        function img = draw_markerfield(state,L)
            img = imresize(ones(size(state.val)),L);
            [M,N] = size(state.val);
            
            text_x = (1+L)/2:L:size(img,2);
            text_x = repmat(text_x,[M,1]);
            text_x = text_x(:)-L/3;
            text_y = (1+L)/2:L:size(img,1);
            text_y = repmat(text_y,[1,N]);
            text_y = text_y(:)-L/3;
            
            img = insertText(img,[text_x,text_y],state.val(:),'FontSize',L/2,'BoxOpacity',0);
        end
        
        function img = draw_deltille(state,L)
            
            assert(L>=5,'side length must be no less than 5!');
            
            [M,N] = size(state.val);
            h = ceil(sqrt(3)/2)*L;
            img = zeros(h*M,L*(N+1)/2);
            
            bias = zeros(M,N);
            bias(2:2:end) = bias(2:2:end)-0.5*L;
            TA_x = repmat(0:L:(N-1)*L,[M,1])+bias;
            TA_y = repmat((0:h:h*(M-1))',[1,N]);
            TA = [TA_x(:),TA_y(:)];
            TB = TA+repmat([L,0],[size(TA,1),1]);
            TC = TA+repmat([L/2,h],[size(TA,1),1]);
            img = insertShape(img,'FilledPolygon',[TA,TB,TC]);
        end
    end % end method
    
end

